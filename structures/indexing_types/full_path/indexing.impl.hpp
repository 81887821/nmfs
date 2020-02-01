#ifndef NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_IMPL_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_IMPL_HPP

#include "indexing.hpp"
#include "../../super_object.impl.hpp"
#include "directory_entry.impl.hpp"
#include "metadata.impl.hpp"
#include "../../../local_caches/cache_store.impl.hpp"

namespace nmfs::structures::indexing_types::full_path {

borrower_slice indexing::existing_directory_key(super_object<indexing>& context, std::string_view path) {
    DECLARE_CONST_BORROWER_SLICE(slice, path.data(), path.size());
    return slice;
}

borrower_slice indexing::existing_regular_file_key(super_object<indexing>& context, std::string_view path) {
    return existing_directory_key(context, path);
}

borrower_slice indexing::new_directory_key(super_object<indexing>& context, std::string_view path, metadata_type& metadata) {
    return existing_directory_key(context, path);
}

borrower_slice indexing::new_regular_file_key(super_object<indexing>& context, std::string_view path, metadata_type& metadata) {
    return existing_directory_key(context, path);
}

mode_t indexing::get_type(super_object<indexing>& context, std::string_view path) {
    auto& metadata = context.cache->open(path);
    mode_t mode = metadata.mode;
    context.cache->close(path, metadata);

    return mode;
}

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_IMPL_HPP
