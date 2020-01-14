#ifndef NMFS_STRUCTURES_METADATA_HPP
#define NMFS_STRUCTURES_METADATA_HPP

#include <sys/stat.h>
#include <cstdint>
#include <cstdlib>
#include <string>
#include "../fuse.hpp"
#include "../primitive_types.hpp"
#include "super_object.hpp"
#include "on_disk/metadata.hpp"

namespace nmfs::structures {

class metadata {
public:
    super_object& context;
    std::string path;
    nlink_t link_count;
    uid_t owner;
    gid_t group;
    mode_t mode;
    uint64_t size;
    struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;
    bool dirty = false;

    metadata(super_object& super, std::string path, fuse_context* fuse_context, mode_t mode);
    metadata(super_object& super, std::string path, const on_disk::metadata& on_disk_structure);

    ssize_t write(const byte* buffer, size_t size, off_t offset);
    ssize_t read(byte* buffer, size_t size, off_t offset);
    void truncate(off_t length);
    void sync_metadata();

private:
    [[nodiscard]] constexpr on_disk::metadata to_on_disk_structure() const;
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
