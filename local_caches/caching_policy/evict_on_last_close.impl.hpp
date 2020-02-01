#ifndef NMFS_LOCAL_CACHES_CACHING_POLICY_EVICT_ON_LAST_CLOSE_IMPL_HPP
#define NMFS_LOCAL_CACHES_CACHING_POLICY_EVICT_ON_LAST_CLOSE_IMPL_HPP

#include "evict_on_last_close.hpp"

#include "../../structures/super_object.hpp"
#include "../../structures/metadata.hpp"
#include "../../structures/directory.hpp"

namespace nmfs::caching_policies {
using namespace nmfs::structures;

template<typename indexing>
bool evict_on_last_close<indexing>::is_valid(super_object<indexing>& context, metadata<indexing>& cache) {
    return true;
}

template<typename indexing>
bool evict_on_last_close<indexing>::keep_cache(super_object<indexing>& context, metadata<indexing>& cache) {
    return cache.open_count > 0;
}

template<typename indexing>
bool evict_on_last_close<indexing>::is_valid(super_object<indexing>& context, directory<indexing>& cache) {
    return true;
}

template<typename indexing>
bool evict_on_last_close<indexing>::keep_cache(super_object<indexing>& context, directory<indexing>& cache) {
    return cache.directory_metadata.open_count > 0;
}

}

#endif //NMFS_LOCAL_CACHES_CACHING_POLICY_EVICT_ON_LAST_CLOSE_IMPL_HPP
