#ifndef NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_DIRECTORY_ENTRY_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_DIRECTORY_ENTRY_HPP

#include "../../directory_entry.hpp"
#include "indexing.hpp"

namespace nmfs::structures::indexing_types::full_path {

class directory_entry: public nmfs::structures::directory_entry<indexing> {
public:
    inline directory_entry(std::string file_name, const nmfs::structures::metadata<indexing>& metadata);
    inline explicit directory_entry(const byte** buffer);
};

}


#endif //NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_DIRECTORY_ENTRY_HPP
