#ifndef NMFS_LOCAL_CACHES_UTILS_DIRECTORY_OPEN_CONTEXT_HPP
#define NMFS_LOCAL_CACHES_UTILS_DIRECTORY_OPEN_CONTEXT_HPP

#include "open_context.hpp"
#include "../../structures/directory.hpp"

namespace nmfs {

template<typename indexing, template<typename> typename lock_type>
class directory_open_context: public open_context<indexing, lock_type> {
public:
    nmfs::structures::directory<indexing>& directory;

    constexpr directory_open_context(std::string_view path, nmfs::structures::directory<indexing>& directory);
    constexpr nmfs::structures::directory<indexing>& unlock_and_release_directory();
};

template<typename indexing, template<typename> typename lock_type>
constexpr directory_open_context<indexing, lock_type>::directory_open_context(std::string_view path, structures::directory<indexing>& directory)
    : open_context<indexing, lock_type>(path, directory.directory_metadata),
      directory(directory) {
}

template<typename indexing, template<typename> typename lock_type>
constexpr nmfs::structures::directory<indexing>& directory_open_context<indexing, lock_type>::unlock_and_release_directory() {
    open_context<indexing, lock_type>::unlock_and_release();
    return directory;
}

}

#endif //NMFS_LOCAL_CACHES_UTILS_DIRECTORY_OPEN_CONTEXT_HPP
