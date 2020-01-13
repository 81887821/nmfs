#ifndef NMFS_STRUCTURES_SUPER_OBJECT_HPP
#define NMFS_STRUCTURES_SUPER_OBJECT_HPP

#include <memory>
#include "../kv_backends/kv_backend.hpp"

namespace nmfs::structures {
using namespace nmfs::kv_backends;

class super_object {
public:
    std::unique_ptr<kv_backend> backend;
};

}

#endif //NMFS_STRUCTURES_SUPER_OBJECT_HPP
