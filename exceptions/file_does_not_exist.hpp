#ifndef NMFS_EXCEPTIONS_FILE_DOES_NOT_EXIST_HPP
#define NMFS_EXCEPTIONS_FILE_DOES_NOT_EXIST_HPP

#include <string>
#include <string_view>
#include "nmfs_exception.hpp"

namespace nmfs::exceptions {

class file_does_not_exist: public nmfs_exception {
public:
    inline explicit file_does_not_exist(std::string_view file_name);
    inline explicit file_does_not_exist(const std::string& file_name);

    [[nodiscard]] inline int error_code() const override;
};

file_does_not_exist::file_does_not_exist(std::string_view file_name): nmfs_exception("File does not exist: " + std::string(file_name)) {
}

file_does_not_exist::file_does_not_exist(const std::string& file_name): nmfs_exception("File does not exist: " + file_name) {
}

int file_does_not_exist::error_code() const {
    return -ENOENT;
}

}

#endif //NMFS_EXCEPTIONS_FILE_DOES_NOT_EXIST_HPP
