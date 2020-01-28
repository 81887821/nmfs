#ifndef NMFS_MAPPER_HPP
#define NMFS_MAPPER_HPP

#include <map>
#include <string>
#include "memory_slices/slice.hpp"

namespace nmfs {

uint64_t next_file_handler;
std::map<std::string, uint64_t> file_handler_map;

}

#endif
