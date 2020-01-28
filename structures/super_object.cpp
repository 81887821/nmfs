#include "super_object.hpp"
#include "../local_caches/cache_store.hpp"
#include "../local_caches/caching_policy/all.hpp"
#include "indexing_types/all.hpp"

using namespace nmfs::structures;

super_object::super_object(std::unique_ptr<kv_backend> backend)
    : backend(std::move(backend)),
      cache(std::make_unique<cache_store<indexing_type, caching_policy>>(*this)) {
}

super_object::~super_object() {
    cache->flush_all();
}
