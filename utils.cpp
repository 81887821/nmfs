#include "utils.hpp"

std::string nmfs::get_parent_directory(std::string &path) {
    if (path.length() == 0) {
        throw std::runtime_error("Invalid Path: path.length() == 0");
    } else {
        for(size_t i; i > 0; i--){
            if(path[i] == PATH_DELIMITER){
                return path.substr(0, i);
            }
        }

        if(path[0] == PATH_DELIMITER){
            return path.substr(0,1); // root directory
        }

        throw std::runtime_error("Invalid Path : No delimiter on path");
    }
}

std::string nmfs::get_filename(std::string &path) {
    if (path.length() == 0) {
        throw std::runtime_error("Invalid Path: path.length() == 0");
    } else {
        for(size_t i; i > 0; i--){
            if(path[i] == PATH_DELIMITER){
                return path.substr(i + 1, path.length());
            }
        }

        if(path[0] == PATH_DELIMITER){
            return path.substr(1, path.length()); // root directory
        }

        throw std::runtime_error("Invalid Path : No delimiter on path");
    }
}