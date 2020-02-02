#ifndef NMFS_LOCAL_CACHES_UTILS_NO_LOCK_HPP
#define NMFS_LOCAL_CACHES_UTILS_NO_LOCK_HPP

namespace nmfs {

template<typename mutex_type>
class no_lock {
public:
    constexpr explicit no_lock(mutex_type& mutex);

    constexpr void lock();
    constexpr void unlock();
};

template<typename mutex_type>
constexpr no_lock<mutex_type>::no_lock(mutex_type& mutex) {
}

template<typename mutex_type>
constexpr void no_lock<mutex_type>::lock() {
}

template<typename mutex_type>
constexpr void no_lock<mutex_type>::unlock() {
}

}

#endif //NMFS_LOCAL_CACHES_UTILS_NO_LOCK_HPP
