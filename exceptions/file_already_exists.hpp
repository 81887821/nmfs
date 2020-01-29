#ifndef NMFS_EXCEPTIONS_FILE_ALREADY_EXISTS_HPP
#define NMFS_EXCEPTIONS_FILE_ALREADY_EXISTS_HPP

#include <string>
#include <string_view>
#include "nmfs_exception.hpp"

namespace nmfs::exceptions {

class file_already_exist: public nmfs_exception {
public:
    inline explicit file_already_exist(std::string_view file_name);
    inline explicit file_already_exist(const std::string& file_name);

    [[nodiscard]] inline int error_code() const override;
};

file_already_exist::file_already_exist(std::string_view file_name): nmfs_exception("File already exists: " + std::string(file_name)) {
}

file_already_exist::file_already_exist(const std::string& file_name): nmfs_exception("File already exists: " + file_name) {
}

int file_already_exist::error_code() const {
    return -EEXIST;
}

}

#endif //NMFS_EXCEPTIONS_FILE_ALREADY_EXISTS_HPP
