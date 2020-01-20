#ifndef NMFS_KV_BACKENDS_EXCEPTIONS_BACKEND_INITIALIZATION_FAILURE_HPP
#define NMFS_KV_BACKENDS_EXCEPTIONS_BACKEND_INITIALIZATION_FAILURE_HPP

#include "generic_kv_api_failure.hpp"

namespace nmfs::kv_backends::exceptions {

class backend_initialization_failure: public generic_kv_api_failure {
public:
    inline backend_initialization_failure(const std::string& message, int error);
};

backend_initialization_failure::backend_initialization_failure(const std::string& message, int error): generic_kv_api_failure(message, error) {
}

}

#endif //NMFS_KV_BACKENDS_EXCEPTIONS_BACKEND_INITIALIZATION_FAILURE_HPP
