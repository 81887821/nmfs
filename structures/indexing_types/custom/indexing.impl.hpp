#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_IMPL_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_IMPL_HPP

#include <functional>

#include "indexing.hpp"
#include "../../super_object.impl.hpp"
#include "../../../local_caches/cache_store.impl.hpp"
#include "directory_entry.impl.hpp"
#include "metadata.impl.hpp"

namespace nmfs::structures::indexing_types::custom {

borrower_slice indexing::existing_directory_key(super_object<indexing>& context, std::string_view path) {
    DECLARE_CONST_BORROWER_SLICE(slice, path.data(), path.size());
    return slice;
}

owner_slice indexing::existing_regular_file_key(super_object<indexing>& context, std::string_view path) {
    std::string_view parent_path = get_parent_directory(path);
    std::string_view file_name = get_filename(path);
    auto open_context = context.cache->open_directory<std::shared_lock>(parent_path);
    auto& entry = open_context.directory.get_entry(file_name);
    auto key = owner_slice(sizeof(uuid_t));

    std::copy(entry.uuid, entry.uuid + sizeof(uuid_t), key.data());
    return key;
}

borrower_slice indexing::new_directory_key(super_object<indexing>& context, std::string_view path, metadata_type& metadata) {
    return existing_directory_key(context, path);
}

borrower_slice indexing::new_regular_file_key(super_object<indexing>& context, std::string_view path, metadata_type& metadata) {
    return borrower_slice(metadata.data_key_base.data(), metadata.data_key_base.size());
}

mode_t indexing::get_type(super_object<indexing>& context, std::string_view path) {
    if (path == "/") {
        return S_IFDIR;
    } else {
        std::string_view parent_path = get_parent_directory(path);
        std::string_view file_name = get_filename(path);
        auto open_context = context.cache->open_directory<std::shared_lock>(parent_path);
        auto& entry = open_context.directory.get_entry(file_name);

        return entry.type;
    }
}

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_IMPL_HPP
