#ifndef NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_DIRECTORY_ENTRY_IMPL_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_DIRECTORY_ENTRY_IMPL_HPP

#include "../../directory_entry.impl.hpp"
#include "indexing.impl.hpp"

namespace nmfs::structures::indexing_types::full_path {

directory_entry::directory_entry(std::string file_name, const nmfs::structures::metadata<indexing>& metadata)
    : nmfs::structures::directory_entry<indexing>(std::move(file_name), metadata) {
}

directory_entry::directory_entry(const byte** buffer)
    : nmfs::structures::directory_entry<indexing>(buffer) {
}

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_DIRECTORY_ENTRY_IMPL_HPP
