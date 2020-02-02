#ifndef NMFS_LOCAL_CACHES_UTILS_OPEN_CONTEXT_HPP
#define NMFS_LOCAL_CACHES_UTILS_OPEN_CONTEXT_HPP

#include <string_view>
#include <shared_mutex>
#include "../../structures/metadata.hpp"

namespace nmfs {

template<typename indexing, template<typename> typename lock_type>
class open_context {
public:
    std::string_view path;
    nmfs::structures::metadata<indexing>& metadata;
    lock_type<std::shared_mutex> lock;

    constexpr open_context(std::string_view path, nmfs::structures::metadata<indexing>& metadata);
    open_context(open_context&& other) noexcept = default;
    inline ~open_context();
    constexpr nmfs::structures::metadata<indexing>& unlock_and_release();

private:
    bool released = false;
};

template<typename indexing, template<typename> typename lock_type>
constexpr open_context<indexing, lock_type>::open_context(std::string_view path, nmfs::structures::metadata<indexing>& metadata)
    : path(path),
      metadata(metadata),
      lock(metadata.mutex) {
    metadata.open_count++;
}

template<typename indexing, template<typename> typename lock_type>
open_context<indexing, lock_type>::~open_context() {
    if (!released) {
        metadata.open_count--;
    }
}

template<typename indexing, template<typename> typename lock_type>
constexpr nmfs::structures::metadata<indexing>& open_context<indexing, lock_type>::unlock_and_release() {
    lock.unlock();
    released = true;
    return metadata;
}

}

#endif //NMFS_LOCAL_CACHES_UTILS_OPEN_CONTEXT_HPP
