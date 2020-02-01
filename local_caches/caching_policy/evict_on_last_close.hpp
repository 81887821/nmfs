#ifndef NMFS_LOCAL_CACHES_EVICT_POLICIES_EVICT_ON_LAST_CLOSE_HPP
#define NMFS_LOCAL_CACHES_EVICT_POLICIES_EVICT_ON_LAST_CLOSE_HPP

#include "../../structures/super_object.hpp"
#include "../../structures/metadata.hpp"
#include "../../structures/directory.hpp"

namespace nmfs::caching_policies {
using namespace nmfs::structures;

template<typename indexing>
class evict_on_last_close {
public:
    static bool is_valid(super_object<indexing>& context, metadata<indexing>& cache);
    static bool keep_cache(super_object<indexing>& context, metadata<indexing>& cache);
    static bool is_valid(super_object<indexing>& context, directory<indexing>& cache);
    static bool keep_cache(super_object<indexing>& context, directory<indexing>& cache);
};

}

#endif //NMFS_LOCAL_CACHES_EVICT_POLICIES_EVICT_ON_LAST_CLOSE_HPP
