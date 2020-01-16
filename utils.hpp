#ifndef NMFS_UTILS_HPP
#define NMFS_UTILS_HPP

#include <string>
#include <cstring>
#include <stdexcept>
#include <uuid/uuid.h>
#include "memory_slices/owner_slice.hpp"

namespace nmfs {

constexpr char path_delimiter = '/';

std::string get_parent_directory(const std::string& path);
std::string get_filename(const std::string& path);

std::unique_ptr<nmfs::slice> make_key(const char* path, uint8_t key_mode);
std::string generate_uuid();
}

#endif
