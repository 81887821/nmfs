#ifndef NMFS_STRUCTURES_SUPER_OBJECT_IMPL_HPP
#define NMFS_STRUCTURES_SUPER_OBJECT_IMPL_HPP

#include "super_object.hpp"
#include "indexing_types/all.impl.hpp"
#include "../local_caches/caching_policy/all.impl.hpp"

namespace nmfs::structures {

template<typename indexing>
super_object<indexing>::super_object(std::unique_ptr<kv_backend> backend)
    : backend(std::move(backend)),
      cache(std::make_unique<cache_store<indexing, caching_policy>>(*this)) {
}

template<typename indexing>
super_object<indexing>::~super_object() {
    cache->flush_all();
}

}

#endif //NMFS_STRUCTURES_SUPER_OBJECT_IMPL_HPP
