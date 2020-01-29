#ifndef NMFS_EXCEPTIONS_NMFS_EXCEPTION_HPP
#define NMFS_EXCEPTIONS_NMFS_EXCEPTION_HPP

#include <stdexcept>
#include <cerrno>

namespace nmfs::exceptions {

class nmfs_exception: public std::runtime_error {
public:
    inline explicit nmfs_exception(const std::string& message);
    [[nodiscard]] inline virtual int error_code() const;
};

nmfs_exception::nmfs_exception(const std::string& message): runtime_error(message) {
}

int nmfs_exception::error_code() const {
    return -EIO;
}

}

#endif //NMFS_EXCEPTIONS_NMFS_EXCEPTION_HPP
