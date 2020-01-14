#ifndef NMFS_UTILS_HPP
#define NMFS_UTILS_HPP

#include <string>
#include <stdexcept>

namespace nmfs {

constexpr char path_delimiter = '/';

std::string get_parent_directory(const std::string& path);
std::string get_filename(const std::string& path);

}

#endif
