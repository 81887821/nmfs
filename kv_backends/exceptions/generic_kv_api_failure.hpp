#ifndef NMFS_KV_BACKENDS_EXCEPTIONS_GENERIC_KV_API_FAILURE_HPP
#define NMFS_KV_BACKENDS_EXCEPTIONS_GENERIC_KV_API_FAILURE_HPP

#include "kv_backend_exception.hpp"

namespace nmfs::kv_backends::exceptions {

class generic_kv_api_failure: public kv_backend_exception {
public:
    inline generic_kv_api_failure(const std::string& message, int error);

    [[nodiscard]] constexpr int get_error();

private:
    int error;
};

generic_kv_api_failure::generic_kv_api_failure(const std::string& message, int error): kv_backend_exception(message), error(error) {
}

constexpr int generic_kv_api_failure::get_error() {
    return error;
}

}

#endif //NMFS_KV_BACKENDS_EXCEPTIONS_GENERIC_KV_API_FAILURE_HPP
