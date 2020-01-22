#ifndef NMFS_EXCEPTIONS_FILE_DOES_NOT_EXIST_HPP
#define NMFS_EXCEPTIONS_FILE_DOES_NOT_EXIST_HPP

#include <string>
#include <string_view>
#include "nmfs_exceptions.hpp"

namespace nmfs::exceptions {

class file_does_not_exist: public nmfs_exceptions {
public:
    inline explicit file_does_not_exist(std::string_view file_name);
    inline explicit file_does_not_exist(const std::string& file_name);
};

file_does_not_exist::file_does_not_exist(std::string_view file_name): nmfs_exceptions("File does not exist: " + std::string(file_name)) {
}

file_does_not_exist::file_does_not_exist(const std::string& file_name): nmfs_exceptions("File does not exist: " + file_name) {
}

}

#endif //NMFS_EXCEPTIONS_FILE_DOES_NOT_EXIST_HPP
