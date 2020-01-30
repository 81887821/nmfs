#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_HPP

#include <functional>
#include <string_view>
#include "../../super_object.hpp"
#include "../../../utils.hpp"
#include "../../../memory_slices/borrower_slice.hpp"
#include "../../../local_caches/cache_store.hpp"
#include "directory_entry.hpp"
#include "metadata.hpp"

namespace nmfs::structures::indexing_types::custom {

class indexing {
public:
    using directory_entry_type = nmfs::structures::indexing_types::custom::directory_entry;
    using metadata_type = nmfs::structures::indexing_types::custom::metadata;

    static inline borrower_slice existing_directory_key(super_object& context, std::string_view path) {
        DECLARE_CONST_BORROWER_SLICE(slice, path.data(), path.size());
        return slice;
    }

    static inline owner_slice existing_regular_file_key(super_object& context, std::string_view path) {
        std::string_view parent_path = get_parent_directory(path);
        std::string_view file_name = get_filename(path);
        auto& parent_directory = context.cache->open_directory(parent_path);
        auto entry = parent_directory.get_entry(file_name);
        auto key = owner_slice(sizeof(uuid_t));

        std::copy(entry.uuid, entry.uuid + sizeof(uuid_t), key.data());
        context.cache->close_directory(parent_path, parent_directory);
        return key;
    }

    static inline borrower_slice new_directory_key(super_object& context, std::string_view path, metadata_type& metadata) {
        return existing_directory_key(context, path);
    }

    static inline borrower_slice new_regular_file_key(super_object& context, std::string_view path, metadata_type& metadata) {
        return borrower_slice(metadata.data_key_base.data(), metadata.data_key_base.size());
    }

    static inline mode_t get_type(super_object& context, std::string_view path) {
        std::string_view parent_path = get_parent_directory(path);
        std::string_view file_name = get_filename(path);
        auto& parent_directory = context.cache->open_directory(parent_path);
        auto entry = parent_directory.get_entry(file_name);

        return entry.type;
    }
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_HPP
