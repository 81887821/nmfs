#ifndef NMFS_LOCAL_CACHES_CACHING_POLICY_HOLD_CLOSED_CACHE_FOR_HPP
#define NMFS_LOCAL_CACHES_CACHING_POLICY_HOLD_CLOSED_CACHE_FOR_HPP

#include <chrono>
#include "../../structures/super_object.hpp"
#include "../../structures/metadata.hpp"
#include "../../structures/directory.hpp"

namespace nmfs::caching_policies {
using namespace nmfs::structures;

template<typename indexing, int seconds>
class hold_closed_cache_for {
public:
    static constexpr auto valid_duration = std::chrono::seconds(seconds);

    static bool is_valid(super_object<indexing>& context, metadata<indexing>& cache);
    static bool keep_cache(super_object<indexing>& context, metadata<indexing>& cache);
    static bool is_valid(super_object<indexing>& context, directory<indexing>& cache);
    static bool keep_cache(super_object<indexing>& context, directory<indexing>& cache);
};

}

#endif //NMFS_LOCAL_CACHES_CACHING_POLICY_HOLD_CLOSED_CACHE_FOR_HPP
