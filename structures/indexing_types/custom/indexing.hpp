#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_HPP

#include <string_view>
#include "../../../utils.hpp"
#include "../../../memory_slices/borrower_slice.hpp"
#include "../../super_object.hpp"
#include "../../../local_caches/cache_store.hpp"
#include "directory_entry.hpp"
#include "metadata.hpp"
#include "on_disk/metadata.hpp"

namespace nmfs::structures::indexing_types::custom {

class indexing {
public:
    using directory_entry_type = nmfs::structures::indexing_types::custom::directory_entry;
    using metadata_type = nmfs::structures::indexing_types::custom::metadata;
    using on_disk_metadata_type = nmfs::structures::indexing_types::custom::on_disk::metadata;

    static inline borrower_slice existing_directory_key(super_object<indexing>& context, std::string_view path);
    static inline owner_slice existing_regular_file_key(super_object<indexing>& context, std::string_view path);
    static inline borrower_slice new_directory_key(super_object<indexing>& context, std::string_view path, metadata_type& metadata);
    static inline borrower_slice new_regular_file_key(super_object<indexing>& context, std::string_view path, metadata_type& metadata);
    static inline mode_t get_type(super_object<indexing>& context, std::string_view path);
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_HPP
