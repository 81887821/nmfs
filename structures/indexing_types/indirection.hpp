#ifndef NMFS_STRUCTURES_INDEXING_TYPES_INDIRECTION_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_INDIRECTION_HPP

#include <string_view>
#include "../super_object.hpp"
#include "../../memory_slices/owner_slice.hpp"

namespace nmfs::indexing_types {

// TODO
class indirection {
    using slice_type = nmfs::owner_slice;
    using directory_content_type = nullptr_t;

    static inline slice_type make_key(super_object& context, const std::string_view& path) {
        throw std::runtime_error("Not implemented");
    }

    static inline on_disk_size_type serialize_directory_content(byte* buffer, const directory_content_type& content) {
        throw std::runtime_error("Not implemented");
    }

    static inline std::tuple<directory_content_type, on_disk_size_type> parse_directory_content(byte* buffer) {
        throw std::runtime_error("Not implemented");
    }

    static inline size_t get_content_size(const directory_content_type& content) {
        throw std::runtime_error("Not implemented");
    }

    static inline int fill_content(const directory_content_type& content, const fuse_directory_filler& filler) {
        throw std::runtime_error("Not implemented");
    }
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_INDIRECTION_HPP
