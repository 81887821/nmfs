#ifndef NMFS_LOCAL_CACHES_CACHE_STORE_IMPL_HPP
#define NMFS_LOCAL_CACHES_CACHE_STORE_IMPL_HPP

#include "cache_store.hpp"
#include "../structures/super_object.hpp"
#include "../logger/log.hpp"
#include "../exceptions/type_not_supported.hpp"
#include "utils/no_lock.hpp"

namespace nmfs {

template<typename indexing, typename caching_policy>
cache_store<indexing, caching_policy>::cache_store(super_object<indexing>& context)
    : context(context),
      background_worker(std::bind(&cache_store::background_worker_main, this)) {
}

template<typename indexing, typename caching_policy>
cache_store<indexing, caching_policy>::~cache_store() {
    stop_background_worker = true;
    background_worker.join();
}

template<typename indexing, typename caching_policy>
template<template<typename> typename lock_type>
inline open_context<indexing, lock_type> cache_store<indexing, caching_policy>::open(std::string_view path) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    auto key_generator = std::function(indexing::existing_regular_file_key);
    auto iterator = open(path, key_generator);
    return open_context<indexing, lock_type>(iterator->first, iterator->second);
}

template<typename indexing, typename caching_policy>
template<template<typename> typename lock_type>
open_context<indexing, lock_type> cache_store<indexing, caching_policy>::create(std::string_view path, uid_t owner, gid_t group, mode_t mode) {
    log::information(log_locations::cache_store_operation) << std::showbase << __func__ << "(path = " << path << ", owner = " << owner << ", group = " << group << ", mode = " << std::oct << mode << ")\n";

    auto shared_cache_lock = std::shared_lock(cache_mutex);
    bool does_exist = cache.contains(path);

    shared_cache_lock.unlock();

    if (does_exist) {
        throw nmfs::exceptions::file_already_exist(path);
    } else {
        auto temporary_metadata = metadata_type(context, owner_slice(0), owner, group, mode);
        auto do_create = [this, path, owner, group, mode, &temporary_metadata]<typename slice_type>(slice_type key) {
            if (context.backend->exist(key)) {
                throw nmfs::exceptions::file_already_exist(path);
            } else {
                auto emplace_result = cache.emplace(
                    std::string(path),
                    std::move(temporary_metadata)
                );
                return emplace_result.first;
            }
        };

        if (S_ISDIR(mode)) {
            auto key = indexing::new_directory_key(context, path, temporary_metadata);
            temporary_metadata.key = owner_slice(key);

            auto lock = std::scoped_lock(directory_cache_mutex, cache_mutex);
            auto iterator = do_create(std::move(key));
            return open_context<indexing, lock_type>(iterator->first, iterator->second);
        } else if (S_ISREG(mode)) {
            auto key = indexing::new_regular_file_key(context, path, temporary_metadata);
            temporary_metadata.key = owner_slice(key);

            auto lock = std::unique_lock(cache_mutex);
            auto iterator = do_create(std::move(key));
            lock.unlock();
            return open_context<indexing, lock_type>(iterator->first, iterator->second);
        } else {
            throw nmfs::exceptions::type_not_supported(mode);
        }
    }
}

template<typename indexing, typename caching_policy>
template<template<typename> typename lock_type>
inline void cache_store<indexing, caching_policy>::close(open_context<indexing, lock_type> open_context) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << open_context.path << ")\n";
    open_context.~open_context(); //TODO ??
    drop_if_policy_requires(open_context.path, open_context.metadata);
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::drop_if_policy_requires(std::string_view path, metadata<indexing>& metadata) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    if (!caching_policy::keep_cache(context, metadata)) {
        auto lock = std::unique_lock(cache_mutex);
        auto iterator = cache.find(path);
        if (iterator != cache.end()) {
            assert(&(iterator->second) == &metadata); // assert if path and metadata is different
            cache.erase(iterator);
        } else {
            // TODO: Error handling - there is no cache with given path
        }
    }
}

template<typename indexing, typename caching_policy>
template<template<typename> typename lock_type>
void cache_store<indexing, caching_policy>::remove(open_context<indexing, lock_type> open_context) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << open_context.path << ")\n";
    auto& metadata = open_context.metadata;

    if (metadata.open_count > 1) {
        log::warning(log_locations::cache_store_operation) << __func__ << "(path = " << open_context.path << "): Removing opened metadata. open_count = " << metadata.open_count << '\n';
    }

    metadata.remove();
    open_context.unlock_and_release();

    auto lock = std::unique_lock(cache_mutex);
    cache.erase(cache.find(open_context.path));
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::move(std::string_view old_path, std::string_view new_path) {
    log::information(log_locations::cache_store_operation) << __func__ << "(old_path = " << old_path << ", new_path = " << new_path << ")\n";
    auto& metadata = reinterpret_cast<metadata_type&>(open<no_lock>(old_path).unlock_and_release()); // To ensure metadata is in cache
    auto cache_lock = std::unique_lock(cache_mutex);
    auto metadata_iterator = cache.find(old_path);
    cache_lock.unlock();

    if (metadata.open_count > 1) {
        log::warning(log_locations::cache_store_operation) << __func__ << ": Renaming opened file. open_count = " << metadata.open_count << '\n';
    }

    auto new_metadata_key = indexing::new_directory_key(context, new_path, metadata);
    auto new_metadata = metadata_type(std::move(metadata), owner_slice(std::move(new_metadata_key)));

    cache_lock.lock();
    auto emplace_result = cache.emplace(
        new_path,
        std::move(new_metadata)
    );
    cache_lock.unlock();
    auto& emplaced_new_metadata = emplace_result.first->second;

    if (metadata.key != emplaced_new_metadata.key) {
        metadata.remove();
    }

    cache.erase(metadata_iterator);
}

template<typename indexing, typename caching_policy>
mode_t cache_store<indexing, caching_policy>::get_type(std::string_view path) {
    return indexing::get_type(context, path);
}

template<typename indexing, typename caching_policy>
template<template<typename> typename lock_type>
directory_open_context<indexing, lock_type> cache_store<indexing, caching_policy>::open_directory(std::string_view path) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    auto directory_shared_lock = std::shared_lock(directory_cache_mutex);
    auto iterator = directory_cache.find(path);

    directory_shared_lock.unlock();

    if (iterator != directory_cache.end()) {
        auto& directory = iterator->second;

        if (caching_policy::is_valid(context, directory)) {
            directory.directory_metadata.open_count++;
            return directory_open_context<indexing, lock_type>(path, directory);
        } else {
            // Drop directory cache and reopen
            auto directory_unique_lock = std::unique_lock(directory_cache_mutex);
            directory_cache.erase(iterator);
        }
    }

    // If directory doesn't exist, an exception will be thrown from open
    auto key_generator = std::function(indexing::existing_directory_key);
    auto metadata_iterator = open(path, key_generator);
    metadata<indexing>& directory_metadata = metadata_iterator->second;

    auto directory_unique_lock = std::unique_lock(directory_cache_mutex);
    auto emplace_result = directory_cache.emplace(
        std::string(path),
        directory_metadata
    );
    directory_unique_lock.unlock();
    return directory_open_context<indexing, lock_type>(emplace_result.first->first, emplace_result.first->second);
}

template<typename indexing, typename caching_policy>
template<template<typename> typename lock_type>
directory_open_context<indexing, lock_type> cache_store<indexing, caching_policy>::create_directory(std::string_view path, uid_t owner, gid_t group, mode_t mode) {
    log::information(log_locations::cache_store_operation) << std::showbase << __func__ << "(path = " << path << ", owner = " << owner << ", group = " << group << ", mode = " << std::oct << mode << ")\n";
    // If directory exists, an exception will be thrown from create
    metadata<indexing>& directory_metadata = create<no_lock>(path, owner, group, mode).unlock_and_release();
    auto lock = std::unique_lock(directory_cache_mutex);
    auto emplace_result = directory_cache.emplace(
        std::string(path),
        directory_metadata
    );
    lock.unlock();
    return directory_open_context<indexing, lock_type>(emplace_result.first->first, emplace_result.first->second);
}

template<typename indexing, typename caching_policy>
template<template<typename> typename lock_type>
void cache_store<indexing, caching_policy>::close_directory(directory_open_context<indexing, lock_type> directory_open_context) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << directory_open_context.path << ")\n";
    directory_open_context.~directory_open_context(); // TODO: ??

    if (!caching_policy::keep_cache(context, directory_open_context.directory)) {
        auto lock = std::unique_lock(directory_cache_mutex);
        auto iterator = directory_cache.find(directory_open_context.path);
        if (iterator != directory_cache.end()) {
            assert(&(iterator->second) == &directory_open_context.directory); // assert if path and directory is different
            directory_cache.erase(iterator);
            lock.unlock();
            drop_if_policy_requires(directory_open_context.path, directory_open_context.metadata);
        } else {
            // TODO: Error handling - there is no cache with given path
        }
    }
}

template<typename indexing, typename caching_policy>
template<template<typename> typename lock_type>
void cache_store<indexing, caching_policy>::remove_directory(directory_open_context<indexing, lock_type> directory_open_context) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << directory_open_context.path << ")\n";
    auto path = directory_open_context.path;
    auto& directory = directory_open_context.directory;
    metadata<indexing>& directory_metadata = directory.directory_metadata;
    directory_metadata.open_count--;

    if (directory_metadata.open_count > 0) {
        log::warning(log_locations::cache_store_operation) << __func__ << "(path = " << path << "): Removing opened directory. open_count = " << directory_metadata.open_count << '\n';
    }

    directory.remove();
    directory_open_context.unlock_and_release_directory();

    auto lock = std::scoped_lock(directory_cache_mutex, cache_mutex);
    directory_cache.erase(directory_cache.find(path));
    cache.erase(cache.find(path));
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::move_directory(std::string_view old_path, std::string_view new_path) {
    log::information(log_locations::cache_store_operation) << __func__ << "(old_path = " << old_path << ", new_path = " << new_path << ")\n";
    auto& directory = open_directory<no_lock>(old_path).unlock_and_release_directory(); // To ensure directory is in cache
    auto& directory_metadata = dynamic_cast<metadata_type&>(directory.directory_metadata);
    auto directory_iterator = directory_cache.find(old_path);
    auto metadata_iterator = cache.find(old_path);

    if (directory_metadata.open_count > 1) {
        log::warning(log_locations::cache_store_operation) << __func__ << ": Renaming opened directory. open_count = " << directory_metadata.open_count << '\n';
    }

    auto new_metadata_key = indexing::new_directory_key(context, new_path, directory_metadata);
    auto new_metadata = metadata_type(std::move(directory_metadata), owner_slice(std::move(new_metadata_key)));
    auto cache_lock = std::unique_lock(cache_mutex);
    cache.emplace(
        new_path,
        std::move(new_metadata)
    );
    cache_lock.unlock();

    auto new_directory = nmfs::structures::directory<indexing>(std::move(directory), old_path, new_path);
    auto directory_cache_lock = std::unique_lock(directory_cache_mutex);
    auto directory_emplace_result = directory_cache.emplace(
        new_path,
        std::move(new_directory)
    );
    auto& emplaced_new_directory = directory_emplace_result.first->second;

    if (emplaced_new_directory.directory_metadata.key != directory_metadata.key) {
        directory_metadata.remove();
    }

    directory_cache.erase(directory_iterator);
    cache.erase(metadata_iterator);
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::flush_all() const {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";
    flush_directories();
    flush_regular_files();
}

template<typename indexing, typename caching_policy>
template<typename slice_type>
typename std::map<std::string, typename cache_store<indexing, caching_policy>::metadata_type, std::less<>>::iterator cache_store<indexing, caching_policy>::open(std::string_view path, std::function<slice_type(super_object<indexing>& , std::string_view)> key_generator) {
    auto cache_shared_lock = std::shared_lock(cache_mutex);
    auto iterator = cache.find(path);

    cache_shared_lock.unlock();

    if (iterator != cache.end()) {
        metadata<indexing>& metadata = iterator->second;
        auto metadata_lock = std::unique_lock(*metadata.mutex);

        if (!caching_policy::is_valid(context, metadata)) {
            metadata.reload();
        }
        return iterator;
    } else {
        slice_type key = key_generator(context, path);
        try {
            owner_slice value = context.backend->get(key);
            auto on_disk_metadata = reinterpret_cast<on_disk::metadata*>(value.data());

            auto cache_unique_lock = std::unique_lock(cache_mutex);
            auto emplace_result = cache.emplace(
                std::string(path),
                metadata_type(context, owner_slice(std::move(key)), on_disk_metadata)
            );
            return emplace_result.first;
        } catch (kv_backends::exceptions::key_does_not_exist& e) {
            throw nmfs::exceptions::file_does_not_exist(path);
        }
    }
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::background_worker_main() {
    const int fail_threshold = 5;
    const auto flush_interval = std::chrono::seconds(5);
    int cache_lock_fail_count = 0;
    int directory_cache_lock_fail_count = 0;
    auto try_flush = [this, fail_threshold](int& fail_count, std::shared_mutex& mutex, std::function<void()> flush) {
        if (auto lock = std::shared_lock(mutex, std::try_to_lock)) {
            fail_count = 0;
            flush();
        } else {
            fail_count++;

            if (fail_count >= fail_threshold) {
                lock.lock();
                fail_count = 0;
                flush();
            }
        }
    };

    while (!stop_background_worker) {
        auto next_flush = std::chrono::system_clock::now() + flush_interval;

        try_flush(cache_lock_fail_count, cache_mutex, [this]() { flush_regular_files(); });
        try_flush(directory_cache_lock_fail_count, directory_cache_mutex, [this]() { flush_directories(); });

        std::this_thread::sleep_until(next_flush);
    }
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::flush_directories() const {
    for (const auto& cache_entry: directory_cache) {
        const std::string& path = cache_entry.first;
        const auto& directory = cache_entry.second;
        const auto& metadata = directory.directory_metadata;

        if (S_ISDIR(metadata.mode) && metadata.dirty) {
            directory.flush();
        }
    }
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::flush_regular_files() const {
    for (const auto& cache_entry: cache) {
        const std::string& path = cache_entry.first;
        const metadata_type& metadata = cache_entry.second;

        if (S_ISREG(metadata.mode) && metadata.dirty) {
            metadata.flush();
        }
    }
}

}

#endif //NMFS_LOCAL_CACHES_CACHE_STORE_IMPL_HPP
