#ifndef NMFS_EXCEPTIONS_IS_NOT_DIRECTORY_HPP
#define NMFS_EXCEPTIONS_IS_NOT_DIRECTORY_HPP

#include <string>
#include <string_view>
#include "nmfs_exceptions.hpp"

namespace nmfs::exceptions {

/**
 * Exception to be thrown when tried to instantiate nmfs::structures::directory with non-directory metadata
 */
class is_not_directory: public nmfs_exceptions {
public:
    inline explicit is_not_directory();
};

is_not_directory::is_not_directory(): nmfs_exceptions("The given file is not a directory") {
}

}

#endif //NMFS_EXCEPTIONS_IS_NOT_DIRECTORY_HPP
