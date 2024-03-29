#ifndef NMFS_STRUCTURES_METADATA_HPP
#define NMFS_STRUCTURES_METADATA_HPP

#include <sys/stat.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <shared_mutex>
#include <string>
#include "../primitive_types.hpp"
#include "../memory_slices/owner_slice.hpp"
#include "utils/data_object_key.hpp"
#include "on_disk/metadata.hpp"
#include "super_object.hpp"

namespace nmfs::structures {

template<typename indexing>
class metadata {
public:
    super_object<indexing>& context;
    owner_slice key;
    size_t open_count;
    nlink_t link_count;
    uid_t owner;
    gid_t group;
    mode_t mode;
    uint64_t size;
    struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;
    bool valid = true;
    std::chrono::system_clock::time_point last_close;
    mutable bool dirty = false;
    mutable std::shared_ptr<std::shared_mutex> mutex;

    inline metadata(super_object<indexing>& super, owner_slice key, uid_t owner, gid_t group, mode_t mode);
    inline metadata(super_object<indexing>& super, owner_slice key, const on_disk::metadata* on_disk_structure);
    inline metadata(metadata&& other, owner_slice key);
    inline metadata(metadata&& other, owner_slice key, const slice& new_data_key_base);
    metadata(const metadata&) = delete;
    metadata(metadata&& other) noexcept;
    virtual inline ~metadata() = default;

    inline ssize_t write(const byte* buffer, size_t size_to_write, off_t offset);
    inline ssize_t read(byte* buffer, size_t size_to_read, off_t offset) const;
    inline void truncate(off_t new_size);
    /**
     * Write local metadata contents to backend
     */
    virtual void flush() const = 0;
    /**
     * Discard local metadata contents and reload from backend
     */
    virtual void reload() = 0;
    virtual void move_data(const slice& new_data_key_base) = 0;
    inline void remove();
    constexpr struct stat to_stat() const;

protected:
    inline void remove_data_objects(uint32_t index_from, uint32_t index_to);
    virtual utils::data_object_key get_data_object_key(uint32_t index) const = 0;
    virtual inline void to_on_disk_metadata(on_disk::metadata& on_disk_metadata) const;
};

}

#endif //NMFS_STRUCTURES_METADATA_HPP
