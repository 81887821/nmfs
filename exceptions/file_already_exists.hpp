#ifndef NMFS_EXCEPTIONS_FILE_ALREADY_EXISTS_HPP
#define NMFS_EXCEPTIONS_FILE_ALREADY_EXISTS_HPP

#include <string>
#include <string_view>
#include "nmfs_exceptions.hpp"

namespace nmfs::exceptions {

class file_already_exist: public nmfs_exceptions {
public:
    inline explicit file_already_exist(const std::string_view& file_name);
    inline explicit file_already_exist(const std::string& file_name);
};

file_already_exist::file_already_exist(const std::string_view& file_name): nmfs_exceptions("File already exists: " + std::string(file_name)) {
}

file_already_exist::file_already_exist(const std::string& file_name): nmfs_exceptions("File already exists: " + file_name) {
}

}

#endif //NMFS_EXCEPTIONS_FILE_ALREADY_EXISTS_HPP
