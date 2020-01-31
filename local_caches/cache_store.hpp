#ifndef NMFS_LOCAL_CACHES_CACHE_STORE_HPP
#define NMFS_LOCAL_CACHES_CACHE_STORE_HPP

#include <cassert>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <thread>
#include <type_traits>
#include <sys/stat.h>
#include <stdexcept>
#include "../structures/directory.hpp"
#include "../structures/metadata.hpp"
#include "../structures/super_object.hpp"
#include "../memory_slices/owner_slice.hpp"
#include "../memory_slices/borrower_slice.hpp"
#include "../kv_backends/exceptions/key_does_not_exist.hpp"
#include "../exceptions/file_does_not_exist.hpp"
#include "../exceptions/file_already_exists.hpp"

namespace nmfs {
using namespace nmfs::structures;

template<typename indexing, typename caching_policy>
class cache_store {
public:
    using directory_entry_type = typename indexing::directory_entry_type;
    using metadata_type = typename indexing::metadata_type;

    explicit cache_store(super_object& context);
    ~cache_store();

    inline metadata& open(std::string_view path);
    inline metadata& create(std::string_view path, uid_t owner, gid_t group, mode_t mode);
    inline void close(std::string_view path, metadata& metadata);
    inline void drop_if_policy_requires(std::string_view path, metadata& metadata);
    inline void remove(std::string_view path, metadata& metadata);
    inline void move(std::string_view old_path, std::string_view new_path);
    inline mode_t get_type(std::string_view path);

    inline directory<directory_entry_type>& open_directory(std::string_view path);
    inline directory<directory_entry_type>& create_directory(std::string_view path, uid_t owner, gid_t group, mode_t mode);
    inline void close_directory(std::string_view path, directory<directory_entry_type>& directory);
    inline void remove_directory(std::string_view path, directory<directory_entry_type>& directory);
    inline void move_directory(std::string_view old_path, std::string_view new_path);

    inline void flush_all() const;

private:
    super_object& context;
    std::map<std::string, metadata_type, std::less<>> cache;
    std::shared_mutex cache_mutex;
    std::map<std::string, directory<directory_entry_type>, std::less<>> directory_cache;
    std::shared_mutex directory_cache_mutex;
    std::thread background_worker;
    bool stop_background_worker = false;

    template<typename slice_type>
    inline metadata& open(std::string_view path, std::function<slice_type(super_object&, std::string_view)> key_generator);
    inline void background_worker_main();
    inline void flush_directories() const;
    inline void flush_regular_files() const;
};

}

#endif //NMFS_LOCAL_CACHES_CACHE_STORE_HPP
