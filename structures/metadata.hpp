#ifndef NMFS_STRUCTURES_METADATA_HPP
#define NMFS_STRUCTURES_METADATA_HPP

#include <sys/stat.h>
#include <cstdint>
#include <cstdlib>
#include <string>
#include "../fuse.hpp"
#include "../primitive_types.hpp"
#include "../memory_slices/owner_slice.hpp"
#include "on_disk/metadata.hpp"
#include "super_object.fwd.hpp"

namespace nmfs::structures {

class metadata {
public:
    super_object& context;
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
    bool dirty = false;

    metadata(super_object& super, owner_slice key, uid_t owner, gid_t group, mode_t mode);
    metadata(super_object& super, owner_slice key, const on_disk::metadata* on_disk_structure);

    ssize_t write(const byte* buffer, size_t size_to_write, off_t offset);
    ssize_t read(byte* buffer, size_t size_to_read, off_t offset);
    void truncate(off_t new_size);
    void sync();
    void refresh();

private:
    [[nodiscard]] constexpr on_disk::metadata to_on_disk_structure() const;
    void remove_data_objects(uint32_t index_from, uint32_t index_to);
};

constexpr on_disk::metadata metadata::to_on_disk_structure() const {
    return {
        .link_count = static_cast<uint32_t>(link_count),
        .owner = owner,
        .group = group,
        .mode = mode,
        .size = size,
        .atime = atime,
        .mtime = mtime,
        .ctime = ctime,
    };
}

}

#endif //NMFS_STRUCTURES_METADATA_HPP
