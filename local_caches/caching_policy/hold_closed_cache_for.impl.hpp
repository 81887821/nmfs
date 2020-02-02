#ifndef NMFS_LOCAL_CACHES_CACHING_POLICY_HOLD_CLOSED_CACHE_FOR_IMPL_HPP
#define NMFS_LOCAL_CACHES_CACHING_POLICY_HOLD_CLOSED_CACHE_FOR_IMPL_HPP

#include "hold_closed_cache_for.hpp"

#include "../../structures/super_object.hpp"
#include "../../structures/metadata.hpp"
#include "../../structures/directory.hpp"

namespace nmfs::caching_policies {
using namespace nmfs::structures;

template<typename indexing, int seconds>
bool hold_closed_cache_for<indexing, seconds>::is_valid(super_object<indexing>& context, metadata<indexing>& cache) {
    return cache.open_count > 0 || cache.last_close <= std::chrono::system_clock::now() + valid_duration;
}

template<typename indexing, int seconds>
bool hold_closed_cache_for<indexing, seconds>::keep_cache(super_object<indexing>& context, metadata<indexing>& cache) {
    return cache.open_count > 0 || cache.last_close <= std::chrono::system_clock::now() + valid_duration;
}

template<typename indexing, int seconds>
bool hold_closed_cache_for<indexing, seconds>::is_valid(super_object<indexing>& context, directory<indexing>& cache) {
    return cache.directory_metadata.open_count > 0 || cache.directory_metadata.last_close <= std::chrono::system_clock::now() + valid_duration;
}

template<typename indexing, int seconds>
bool hold_closed_cache_for<indexing, seconds>::keep_cache(super_object<indexing>& context, directory<indexing>& cache) {
    return cache.directory_metadata.open_count > 0 || cache.directory_metadata.last_close <= std::chrono::system_clock::now() + valid_duration;
}

}

#endif //NMFS_LOCAL_CACHES_CACHING_POLICY_HOLD_CLOSED_CACHE_FOR_IMPL_HPP
