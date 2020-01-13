#ifndef NMFS_STRUCTURES_ON_DISK_METADATA_HPP
#define NMFS_STRUCTURES_ON_DISK_METADATA_HPP

namespace nmfs::structures::on_disk {

struct metadata {
    uint32_t link_count;
    uint32_t owner;
    uint32_t group;
    uint32_t mode;
    uint64_t size;
    struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;
};

}

#endif //NMFS_STRUCTURES_ON_DISK_METADATA_HPP
