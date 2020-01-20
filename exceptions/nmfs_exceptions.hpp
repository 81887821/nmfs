#ifndef NMFS_EXCEPTIONS_NMFS_EXCEPTIONS_HPP
#define NMFS_EXCEPTIONS_NMFS_EXCEPTIONS_HPP

#include <stdexcept>

namespace nmfs::exceptions {

class nmfs_exceptions: public std::runtime_error {
public:
    inline explicit nmfs_exceptions(const std::string& message);
};

nmfs_exceptions::nmfs_exceptions(const std::string& message): runtime_error(message) {
}

}

#endif //NMFS_EXCEPTIONS_NMFS_EXCEPTIONS_HPP
