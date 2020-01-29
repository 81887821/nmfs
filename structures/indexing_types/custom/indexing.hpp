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

    static inline borrower_slice make_directory_key(super_object& context, std::string_view path) {
        DECLARE_CONST_BORROWER_SLICE(slice, path.data(), path.size());
        return slice;
    }

    static inline owner_slice make_regular_file_key(super_object& context, std::string_view path) {
        throw std::runtime_error("Not implemented yet.");
    }
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_INDEXING_HPP
