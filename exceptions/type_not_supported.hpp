#ifndef NMFS_EXCEPTIONS_TYPE_NOT_SUPPORTED_HPP
#define NMFS_EXCEPTIONS_TYPE_NOT_SUPPORTED_HPP

#include <fcntl.h>
#include "nmfs_exception.hpp"

namespace nmfs::exceptions {

class type_not_supported: public nmfs_exception {
public:
    inline explicit type_not_supported(mode_t mode);
};

type_not_supported::type_not_supported(mode_t mode): nmfs_exception("Unsupported file type: " + std::to_string(mode & S_IFMT)) {

}

}


#endif //NMFS_EXCEPTIONS_TYPE_NOT_SUPPORTED_HPP
