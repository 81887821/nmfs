#ifndef NMFS_LOCAL_CACHES_EVICT_POLICIES_EVICT_ON_LAST_CLOSE_HPP
#define NMFS_LOCAL_CACHES_EVICT_POLICIES_EVICT_ON_LAST_CLOSE_HPP

#include "../../structures/super_object.hpp"
#include "../../structures/metadata.hpp"
#include "../../structures/directory.hpp"

namespace nmfs::caching_policies {

class evict_on_last_close {
public:
    static bool is_valid(super_object& context, metadata& cache) {
        return true;
    }

    static bool keep_cache(super_object& context, metadata& cache) {
        return cache.open_count > 0;
    }

    template<typename indexing>
    static bool is_valid(super_object& context, directory<indexing>& cache) {
        return true;
    }

    template<typename indexing>
    static bool keep_cache(super_object& context, directory<indexing>& cache) {
        return cache.directory_metadata.open_count > 0;
    }
};

}

#endif //NMFS_LOCAL_CACHES_EVICT_POLICIES_EVICT_ON_LAST_CLOSE_HPP
