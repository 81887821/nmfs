#ifndef NMFS_EXCEPTIONS_IS_NOT_DIRECTORY_HPP
#define NMFS_EXCEPTIONS_IS_NOT_DIRECTORY_HPP

#include <string>
#include <string_view>
#include "nmfs_exception.hpp"

namespace nmfs::exceptions {

/**
 * Exception to be thrown when tried to instantiate nmfs::structures::directory with non-directory metadata
 */
class is_not_directory: public nmfs_exception {
public:
    inline explicit is_not_directory();
    [[nodiscard]] inline int error_code() const override;
};

is_not_directory::is_not_directory(): nmfs_exception("The given file is not a directory") {
}

int is_not_directory::error_code() const {
    return -ENOTDIR;
}

}

#endif //NMFS_EXCEPTIONS_IS_NOT_DIRECTORY_HPP
