#ifndef NMFS_LOCAL_CACHES_UTILS_DIRECTORY_OPEN_CONTEXT_HPP
#define NMFS_LOCAL_CACHES_UTILS_DIRECTORY_OPEN_CONTEXT_HPP

#include "open_context.hpp"
#include "../../structures/directory.hpp"

namespace nmfs {

template<typename indexing, template<typename> typename lock_type>
class directory_open_context: public open_context<indexing, lock_type> {
public:
    nmfs::structures::directory<indexing>& directory;

    constexpr directory_open_context(std::string_view path, nmfs::structures::directory<indexing>& directory, bool increase_open_count = false);
    constexpr directory_open_context(directory_open_context&& other) noexcept;
    inline ~directory_open_context();
    constexpr nmfs::structures::directory<indexing>& unlock_and_release_directory();
};

template<typename indexing, template<typename> typename lock_type>
constexpr directory_open_context<indexing, lock_type>::directory_open_context(std::string_view path, structures::directory<indexing>& directory, bool increase_open_count)
    : open_context<indexing, lock_type>(path, directory.directory_metadata, increase_open_count),
      directory(directory) {
}

template<typename indexing, template<typename> typename lock_type>
constexpr directory_open_context<indexing, lock_type>::directory_open_context(directory_open_context&& other) noexcept
    : open_context<indexing, lock_type>(std::move(other)),
      directory(other.directory) {
}

template<typename indexing, template<typename> typename lock_type>
directory_open_context<indexing, lock_type>::~directory_open_context() {
    if (!this->released) {
        log::information(log_locations::cache_store_operation) << __func__ << ": path = " << this->path << '\n';
        this->metadata.open_count--;
        this->lock.unlock();
        this->released = true;
        this->metadata.context.cache->drop_if_policy_requires(this->path, this->directory);
        this->metadata.context.cache->drop_if_policy_requires(this->path, this->metadata);
    }
}

template<typename indexing, template<typename> typename lock_type>
constexpr nmfs::structures::directory<indexing>& directory_open_context<indexing, lock_type>::unlock_and_release_directory() {
    open_context<indexing, lock_type>::unlock_and_release();
    return directory;
}

}

#endif //NMFS_LOCAL_CACHES_UTILS_DIRECTORY_OPEN_CONTEXT_HPP
