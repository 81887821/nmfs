#ifndef NMFS_LOCAL_CACHES_CACHE_STORE_HPP
#include "../local_caches/cache_store.hpp"
#endif //NMFS_LOCAL_CACHES_CACHE_STORE_HPP

#ifndef NMFS_STRUCTURES_SUPER_OBJECT_HPP
#define NMFS_STRUCTURES_SUPER_OBJECT_HPP

#include <memory>
#include "../kv_backends/kv_backend.hpp"
#include "../configuration.hpp"

namespace nmfs::structures {
using namespace nmfs::kv_backends;

class super_object {
public:
    using indexing_type = configuration::indexing_type;
    using caching_policy = configuration::caching_policy;

    const size_t maximum_object_size = 64 * 1024;

    std::unique_ptr<kv_backend> backend;
    cache_store<configuration::indexing_type, configuration::caching_policy> cache;

    explicit inline super_object(std::unique_ptr<kv_backend> backend);
};

inline super_object::super_object(std::unique_ptr<kv_backend> backend): backend(std::move(backend)), cache(*this) {
}

}

#endif //NMFS_STRUCTURES_SUPER_OBJECT_HPP
