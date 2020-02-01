#ifndef NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_HPP

#include <string_view>
#include "../../../primitive_types.hpp"
#include "../../../memory_slices/borrower_slice.hpp"
#include "../../super_object.hpp"
#include "directory_entry.hpp"
#include "metadata.hpp"
#include "../../../local_caches/cache_store.hpp"

namespace nmfs::structures::indexing_types::full_path {

class indexing {
public:
    using directory_entry_type = nmfs::structures::indexing_types::full_path::directory_entry;
    using metadata_type = nmfs::structures::indexing_types::full_path::metadata;

    static inline borrower_slice existing_directory_key(super_object<indexing>& context, std::string_view path);
    static inline borrower_slice existing_regular_file_key(super_object<indexing>& context, std::string_view path);
    static inline borrower_slice new_directory_key(super_object<indexing>& context, std::string_view path, metadata_type& metadata);
    static inline borrower_slice new_regular_file_key(super_object<indexing>& context, std::string_view path, metadata_type& metadata);
    static inline mode_t get_type(super_object<indexing>& context, std::string_view path);
};

}


#endif //NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_HPP
