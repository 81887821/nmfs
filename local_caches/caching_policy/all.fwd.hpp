#ifndef NMFS_LOCAL_CACHES_CACHING_POLICY_ALL_FWD_HPP
#define NMFS_LOCAL_CACHES_CACHING_POLICY_ALL_FWD_HPP

namespace nmfs::caching_policies {

template<typename indexing>
class evict_on_last_close;
template<typename indexing, int seconds>
class hold_closed_cache_for;

}

#endif //NMFS_LOCAL_CACHES_CACHING_POLICY_ALL_FWD_HPP
