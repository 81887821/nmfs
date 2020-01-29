#ifndef NMFS_LOCAL_CACHES_CACHE_STORE_HPP
#define NMFS_LOCAL_CACHES_CACHE_STORE_HPP

#include <cassert>
#include <functional>
#include <map>
#include <string_view>
#include <type_traits>
#include <sys/stat.h>
#include <stdexcept>
#include "../structures/directory.hpp"
#include "../structures/metadata.hpp"
#include "../structures/super_object.hpp"
#include "../memory_slices/owner_slice.hpp"
#include "../memory_slices/borrower_slice.hpp"
#include "../kv_backends/exceptions/key_does_not_exist.hpp"
#include "../exceptions/file_does_not_exist.hpp"
#include "../exceptions/file_already_exists.hpp"

namespace nmfs {
using namespace nmfs::structures;

template<typename indexing, typename caching_policy>
class cache_store {
public:
    using directory_entry_type = typename indexing::directory_entry_type;
    using metadata_type = typename indexing::metadata_type;

    explicit cache_store(super_object& context);

    inline metadata& open(std::string_view path);
    inline metadata& create(std::string_view path, uid_t owner, gid_t group, mode_t mode);
    inline void close(std::string_view path, metadata& metadata);
    inline void drop_if_policy_requires(std::string_view path, metadata& metadata);
    inline void remove(std::string_view path, metadata& metadata);

    inline directory<directory_entry_type>& open_directory(std::string_view path);
    inline directory<directory_entry_type>& create_directory(std::string_view path, uid_t owner, gid_t group, mode_t mode);
    inline void close_directory(std::string_view path, directory<directory_entry_type>& directory);
    inline void remove_directory(std::string_view path, directory<directory_entry_type>& directory);

    inline void flush_all() const;

private:
    super_object& context;
    std::map<std::string, metadata_type, std::less<>> cache;
    std::map<std::string, directory<directory_entry_type>, std::less<>> directory_cache;

    template<typename slice_type>
    inline metadata& open(std::string_view path, std::function<slice_type(super_object&, std::string_view)> key_generator);
};

}

#include "../structures/super_object.hpp"
#include "../logger/log.hpp"
#include "../exceptions/type_not_supported.hpp"

namespace nmfs {

template<typename indexing, typename caching_policy>
cache_store<indexing, caching_policy>::cache_store(super_object& context): context(context) {
}

template<typename indexing, typename caching_policy>
inline metadata& cache_store<indexing, caching_policy>::open(std::string_view path) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    auto key_generator = std::function(indexing::existing_regular_file_key);
    return open(path, key_generator);
}

template<typename indexing, typename caching_policy>
metadata& cache_store<indexing, caching_policy>::create(std::string_view path, uid_t owner, gid_t group, mode_t mode) {
    log::information(log_locations::cache_store_operation) << std::showbase << __func__ << "(path = " << path << ", owner = " << owner << ", group = " << group << ", mode = " << std::oct << mode << ")\n";

    if (cache.contains(path)) {
        throw nmfs::exceptions::file_already_exist(path);
    } else {
        auto temporary_metadata = metadata_type(context, owner_slice(0), owner, group, mode);
        auto do_create = [this, path, owner, group, mode, &temporary_metadata]<typename slice_type>(slice_type key) -> metadata& {
            if (context.backend->exist(key)) {
                throw nmfs::exceptions::file_already_exist(path);
            } else {
                auto emplace_result = cache.emplace(std::make_pair(
                    std::string(path),
                    std::move(temporary_metadata)
                ));
                return emplace_result.first->second;
            }
        };

        if (S_ISDIR(mode)) {
            auto key = indexing::new_directory_key(context, path, temporary_metadata);
            temporary_metadata.key = owner_slice(key);
            return do_create(std::move(key));
        } else if (S_ISREG(mode)) {
            auto key = indexing::new_regular_file_key(context, path, temporary_metadata);
            temporary_metadata.key = owner_slice(key);
            return do_create(std::move(key));
        } else {
            throw nmfs::exceptions::type_not_supported(mode);
        }
    }
}

template<typename indexing, typename caching_policy>
inline void cache_store<indexing, caching_policy>::close(std::string_view path, metadata& metadata) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    metadata.open_count--;
    drop_if_policy_requires(path, metadata);
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::drop_if_policy_requires(std::string_view path, metadata& metadata) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    if (!caching_policy::keep_cache(context, metadata)) {
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
void cache_store<indexing, caching_policy>::remove(std::string_view path, metadata& metadata) {
    log::information(log_locations::cache_store_operation) << __func__ << "( " << path << " )\n";
    metadata.open_count--;

    if (metadata.open_count > 0) {
        log::warning(log_locations::cache_store_operation) << __func__ << "(path = " << path << "): Removing opened metadata. open_count = " << metadata.open_count << '\n';
    }

    metadata.remove();
    cache.erase(cache.find(path));
}

template<typename indexing, typename caching_policy>
directory<typename indexing::directory_entry_type>& cache_store<indexing, caching_policy>::open_directory(std::string_view path) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    auto iterator = directory_cache.find(path);

    if (iterator != directory_cache.end()) {
        auto& directory = iterator->second;

        if (caching_policy::is_valid(context, directory)) {
            directory.directory_metadata.open_count++;
            return directory;
        } else {
            // Drop directory cache and reopen
            directory_cache.erase(iterator);
        }
    }

    // If directory doesn't exist, an exception will be thrown from open
    auto key_generator = std::function(indexing::existing_directory_key);
    metadata& directory_metadata = open(path, key_generator);
    auto emplace_result = directory_cache.emplace(std::make_pair(
        std::string(path),
        directory<directory_entry_type>(directory_metadata)
    ));
    return emplace_result.first->second;
}

template<typename indexing, typename caching_policy>
directory<typename indexing::directory_entry_type>& cache_store<indexing, caching_policy>::create_directory(std::string_view path, uid_t owner, gid_t group, mode_t mode) {
    log::information(log_locations::cache_store_operation) << std::showbase << __func__ << "(path = " << path << ", owner = " << owner << ", group = " << group << ", mode = " << std::oct << mode << ")\n";
    // If directory exists, an exception will be thrown from create
    metadata& directory_metadata = create(path, owner, group, mode);
    auto emplace_result = directory_cache.emplace(std::make_pair(
        std::string(path),
        directory<directory_entry_type>(directory_metadata)
    ));
    return emplace_result.first->second;
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::close_directory(std::string_view path, directory<directory_entry_type>& directory) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    metadata& directory_metadata = directory.directory_metadata;
    directory_metadata.open_count--;

    if (!caching_policy::keep_cache(context, directory)) {
        auto iterator = directory_cache.find(path);
        if (iterator != directory_cache.end()) {
            assert(&(iterator->second) == &directory); // assert if path and directory is different
            directory_cache.erase(iterator);
            drop_if_policy_requires(path, directory_metadata);
        } else {
            // TODO: Error handling - there is no cache with given path
        }
    }
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::remove_directory(std::string_view path, directory<directory_entry_type>& directory) {
    log::information(log_locations::cache_store_operation) << __func__ << "(path = " << path << ")\n";
    metadata& directory_metadata = directory.directory_metadata;
    directory_metadata.open_count--;

    if (directory_metadata.open_count > 0) {
        log::warning(log_locations::cache_store_operation) << __func__ << "(path = " << path << "): Removing opened directory. open_count = " << directory_metadata.open_count << '\n';
    }

    directory.remove();
    directory_cache.erase(directory_cache.find(path));
    cache.erase(cache.find(path));
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::flush_all() const {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";
    for (const auto& directory: directory_cache) {
        directory.second.flush();
    }

    for (const auto& metadata: cache) {
        metadata.second.flush();
    }
}

template<typename indexing, typename caching_policy>
template<typename slice_type>
metadata& cache_store<indexing, caching_policy>::open(std::string_view path, std::function<slice_type(super_object&, std::string_view)> key_generator) {
    auto iterator = cache.find(path);

    if (iterator != cache.end()) {
        metadata& metadata = iterator->second;

        if (!caching_policy::is_valid(context, metadata)) {
            metadata.reload();
        }
        metadata.open_count++;
        return metadata;
    } else {
        slice_type key = key_generator(context, path);
        try {
            owner_slice value = context.backend->get(key);
            auto on_disk_metadata = reinterpret_cast<on_disk::metadata*>(value.data());

            auto emplace_result = cache.emplace(std::make_pair(
                std::string(path),
                metadata_type(context, owner_slice(std::move(key)), on_disk_metadata)
            ));
            return emplace_result.first->second;
        } catch (kv_backends::exceptions::key_does_not_exist& e) {
            throw nmfs::exceptions::file_does_not_exist(path);
        }
    }
}

}

#endif //NMFS_LOCAL_CACHES_CACHE_STORE_HPP
