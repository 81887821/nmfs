#ifndef NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_HPP

#include <string_view>
#include "../../../primitive_types.hpp"
#include "../../super_object.hpp"
#include "../../../memory_slices/borrower_slice.hpp"
#include "../../directory_entry.hpp"
#include "metadata.hpp"

namespace nmfs::structures::indexing_types::full_path {

class indexing {
public:
    using directory_entry_type = nmfs::structures::directory_entry;
    using metadata_type = nmfs::structures::indexing_types::full_path::metadata;

    static inline borrower_slice make_directory_key(super_object& context, std::string_view path) {
        DECLARE_CONST_BORROWER_SLICE(slice, path.data(), path.size());
        return slice;
    }

    static inline borrower_slice make_regular_file_key(super_object& context, std::string_view path) {
        return make_directory_key(context, path);
    }
};

}


#endif //NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_INDEXING_HPP
