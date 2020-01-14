#ifndef NMFS_UTILS_HPP
#define NMFS_UTILS_HPP

#include <string>
#include <stdexcept>

#define PATH_DELIMITER '/'

namespace nmfs {

std::string get_parent_directory(std::string& path);
std::string get_filename(std::string& path);

}

#endif
