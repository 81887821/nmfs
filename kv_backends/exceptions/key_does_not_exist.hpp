#ifndef NMFS_KV_BACKENDS_EXCEPTIONS_KEY_DOES_NOT_EXIST_HPP
#define NMFS_KV_BACKENDS_EXCEPTIONS_KEY_DOES_NOT_EXIST_HPP

#include <string>
#include "kv_backend_exception.hpp"
#include "../../memory_slices/slice.hpp"

namespace nmfs::kv_backends::exceptions {

class key_does_not_exist: public kv_backend_exception {
public:
    inline explicit key_does_not_exist(const slice& key);
};

key_does_not_exist::key_does_not_exist(const slice& key): kv_backend_exception(std::string("Key does not exist: ") + key.to_string()) {
}

}

#endif //NMFS_KV_BACKENDS_EXCEPTIONS_KEY_DOES_NOT_EXIST_HPP
