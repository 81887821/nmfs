#ifndef NMFS_LOCAL_CACHES_CACHE_STORE_HPP
#define NMFS_LOCAL_CACHES_CACHE_STORE_HPP

#include <cassert>
#include <map>
#include <string_view>
#include <type_traits>
#include <sys/stat.h>
#include <stdexcept>
#include "../structures/directory.hpp"
#include "../structures/metadata.hpp"
#include "../structures/super_object.fwd.hpp"
#include "../memory_slices/owner_slice.hpp"
#include "../kv_backends/exceptions/key_does_not_exist.hpp"
#include "../exceptions/file_does_not_exist.hpp"
#include "../exceptions/file_already_exists.hpp"

namespace nmfs {
using namespace nmfs::structures;

template<typename indexing, typename caching_policy>
class cache_store {
public:
    explicit cache_store(super_object& context);

    inline metadata& open(std::string_view path);
    inline metadata& open(std::string_view path, std::string parent_directory_uuid);
    inline metadata& create(std::string_view path, uid_t owner, gid_t group, mode_t mode);
    inline void close(std::string_view path, metadata& metadata);
    inline void drop_if_policy_requires(std::string_view path, metadata& metadata);
    inline void remove(std::string_view path, metadata& metadata);

    inline directory<indexing>& open_directory(std::string_view path);
    inline directory<indexing>& create_directory(std::string_view path, uid_t owner, gid_t group, mode_t mode);
    inline void close_directory(std::string_view path, directory<indexing>& directory);
    inline void remove_directory(std::string_view path, directory<indexing>& directory);

    inline void flush_all() const;

private:
    super_object& context;
    std::map<std::string, metadata, std::less<>> cache;
    std::map<std::string, directory<indexing>, std::less<>> directory_cache;
    std::map<std::string, std::string, std::less<>> uuid_cache; // (path, uuid)
};

}

#include "../structures/super_object.hpp"
#include "../logger/log.hpp"

namespace nmfs {

template<typename indexing, typename caching_policy>
cache_store<indexing, caching_policy>::cache_store(super_object& context): context(context) {
}

template<typename indexing, typename caching_policy>
inline metadata& cache_store<indexing, caching_policy>::open(std::string_view path) {
    log::information(log_locations::cache_store_operation) << __func__ << " for directory metadata()\n";
    auto iterator = cache.find(path);

    if (iterator != cache.end()) {
        metadata& metadata = iterator->second;

        if (!caching_policy::is_valid(context, metadata)) {
            metadata.reload();
        }
        metadata.open_count++;
        return metadata;
    } else {
        typename indexing::slice_type key = indexing::make_key(context, path);
        try {
            owner_slice value = context.backend->get(key);
            auto on_disk_metadata = reinterpret_cast<on_disk::metadata*>(value.data());

            auto emplace_result = cache.emplace(std::make_pair(
                std::string(path),
                metadata(context, owner_slice(std::move(key)), on_disk_metadata)
            ));
            return emplace_result.first->second;
        } catch (kv_backends::exceptions::key_does_not_exist& e) {
            throw nmfs::exceptions::file_does_not_exist(path);
        }
    }
}

template<typename indexing, typename caching_policy>
inline metadata& cache_store<indexing, caching_policy>::open(std::string_view path, std::string parent_directory_uuid) {
    log::information(log_locations::cache_store_operation) << __func__ << " for file metadata()\n";
    auto iterator = cache.find(path);
    std::string *file_uuid;
    if (iterator != cache.end()) {
        metadata& metadata = iterator->second;

        if (!caching_policy::is_valid(context, metadata)) {
            metadata.reload();
        }
        metadata.open_count++;
        return metadata;
    } else {
        auto uuid_cache_iterator = uuid_cache.find(path);
        if(uuid_cache_iterator != uuid_cache.end()) {
            file_uuid = &(uuid_cache_iterator->second);
        } else {
            // create new file_uuid
            file_uuid = new std::string(nmfs::generate_uuid());
            auto uuid_emplace_result = uuid_cache.emplace(std::make_pair(
                std::string(path),
                *file_uuid
            ));
        }

        typename indexing::slice_type key = indexing::make_key(context, path, parent_directory_uuid, *file_uuid); // not just make slice but first find corresponding uuid and if uuid exsit, make key

        try {
            owner_slice value = context.backend->get(key);
            auto on_disk_metadata = reinterpret_cast<on_disk::metadata*>(value.data());
            auto emplace_result = cache.emplace(std::make_pair(
                std::string(path),
                metadata(context, owner_slice(std::move(key)), on_disk_metadata)
            ));
            if(S_ISREG(emplace_result.first->second.mode)) {
                emplace_result.first->second.uuid = std::move(std::string(key.data(), key.size()));
            }
            return emplace_result.first->second;
        } catch (kv_backends::exceptions::key_does_not_exist& e) {
            throw nmfs::exceptions::file_does_not_exist(path);
        }
    }
}

template<typename indexing, typename caching_policy>
metadata& cache_store<indexing, caching_policy>::create(std::string_view path, uid_t owner, gid_t group, mode_t mode) {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";

    typename indexing::slice_type key = indexing::make_key(context, path);
    std::string *file_uuid;

    if (cache.contains(path)) {
        throw nmfs::exceptions::file_already_exist(path);
    } else {
        if(S_ISDIR(mode)) {
            //key = indexing::make_key(context, path);
        } else if (S_ISREG(mode)){
            fuse_context* fuse_context = fuse_get_context();
            auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

            std::string_view parent_path = get_parent_directory(path);
            structures::metadata& parent_directory_metadata = super_object->cache.open(parent_path);

            auto uuid_cache_iterator = uuid_cache.find(path);
            if(uuid_cache_iterator != uuid_cache.end()) {
                file_uuid = &(uuid_cache_iterator->second);
            } else {
                // create new file_uuid
                file_uuid = new std::string(nmfs::generate_uuid(), key.size());
                auto uuid_emplace_result = uuid_cache.emplace(std::make_pair(
                    std::string(path),
                    *file_uuid
                ));
            }

            key = indexing::make_key(context, path, parent_directory_metadata.uuid, *file_uuid);
        } // S_ISREG

        if (context.backend->exist(key)) {
            throw nmfs::exceptions::file_already_exist(path);
        } else {
            auto emplace_result = cache.emplace(std::make_pair(
                std::string(path),
                metadata(context, owner_slice(std::move(key)), owner, group, mode)
            ));
            if(S_ISREG(mode)) {
                emplace_result.first->second.uuid = std::string(key.data());
            }
            return emplace_result.first->second;
        }
    }
}

template<typename indexing, typename caching_policy>
inline void cache_store<indexing, caching_policy>::close(std::string_view path, metadata& metadata) {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";
    metadata.open_count--;
    drop_if_policy_requires(path, metadata);
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::drop_if_policy_requires(std::string_view path, metadata& metadata) {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";
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
    log::information(log_locations::cache_store_operation) << __func__ << "( "<< path <<" )\n";
    metadata.open_count--;

    if (metadata.open_count > 0) {
        log::warning(log_locations::cache_store_operation) << __func__ << "(path = " << path << "): Removing opened metadata. open_count = " << metadata.open_count << '\n';
    }

    metadata.remove();
    cache.erase(cache.find(path));
}

template<typename indexing, typename caching_policy>
directory<indexing>& cache_store<indexing, caching_policy>::open_directory(std::string_view path) {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";
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
    metadata& directory_metadata = open(path);
    auto emplace_result = directory_cache.emplace(std::make_pair(
        std::string(path),
        directory<indexing>(directory_metadata)
    ));
    return emplace_result.first->second;
}

template<typename indexing, typename caching_policy>
directory<indexing>& cache_store<indexing, caching_policy>::create_directory(std::string_view path, uid_t owner, gid_t group, mode_t mode) {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";
    // If directory exists, an exception will be thrown from create
    metadata& directory_metadata = create(path, owner, group, mode);
    auto emplace_result = directory_cache.emplace(std::make_pair(
        std::string(path),
        directory<indexing>(directory_metadata)
    ));
    return emplace_result.first->second;
}

template<typename indexing, typename caching_policy>
void cache_store<indexing, caching_policy>::close_directory(std::string_view path, directory <indexing>& directory) {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";
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
void cache_store<indexing, caching_policy>::remove_directory(std::string_view path, directory <indexing>& directory) {
    log::information(log_locations::cache_store_operation) << __func__ << "()\n";
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

}

#endif //NMFS_LOCAL_CACHES_CACHE_STORE_HPP
