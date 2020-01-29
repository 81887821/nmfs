#ifndef NMFS_KV_BACKENDS_EXCEPTIONS_KV_BACKEND_EXCEPTION_HPP
#define NMFS_KV_BACKENDS_EXCEPTIONS_KV_BACKEND_EXCEPTION_HPP

#include "../../exceptions/nmfs_exception.hpp"

namespace nmfs::kv_backends::exceptions {

class kv_backend_exception: public nmfs::exceptions::nmfs_exception {
public:
    inline explicit kv_backend_exception(const std::string& message);
};

kv_backend_exception::kv_backend_exception(const std::string& message): nmfs::exceptions::nmfs_exception(message) {
}

}


#endif //NMFS_KV_BACKENDS_EXCEPTIONS_KV_BACKEND_EXCEPTION_HPP
