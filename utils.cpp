#include "utils.hpp"

std::string nmfs::get_parent_directory(const std::string& path) {
    if (path.empty()) {
        throw std::runtime_error("Invalid path: path is empty string");
    } else {
        for (size_t i = path.length() - 1; i > 0; i--) {
            if (path[i] == path_delimiter) {
                return path.substr(0, i);
            }
        }

        if (path[0] == path_delimiter) {
            return path.substr(0, 1); // root directory
        }

        throw std::runtime_error("Invalid path: No delimiter on path");
    }
}

std::string nmfs::get_filename(const std::string& path) {
    if (path.empty()) {
        throw std::runtime_error("Invalid path: path is empty string");
    } else {
        for (size_t i = path.length() - 1; i > 0; i--) {
            if (path[i] == path_delimiter) {
                return path.substr(i + 1);
            }
        }

        if (path[0] == path_delimiter) {
            return path.substr(1); // root directory
        }

        throw std::runtime_error("Invalid path: No delimiter on path");
    }
}
