#ifndef NMFS_STRUCTURES_SUPER_OBJECT_HPP
#define NMFS_STRUCTURES_SUPER_OBJECT_HPP

#include <memory>
#include "../kv_backends/kv_backend.hpp"

namespace nmfs::structures {
using namespace nmfs::kv_backends;

class super_object {
public:
    const size_t maximum_object_size = 64 * 1024;

    std::unique_ptr<kv_backend> backend;

    explicit inline super_object(std::unique_ptr<kv_backend> backend);
};

inline super_object::super_object(std::unique_ptr<kv_backend> backend): backend(std::move(backend)) {
}

}

#endif //NMFS_STRUCTURES_SUPER_OBJECT_HPP
