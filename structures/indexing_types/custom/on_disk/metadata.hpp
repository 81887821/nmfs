#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_ON_DISK_METADATA_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_ON_DISK_METADATA_HPP

#include <uuid.h>
#include "../../../on_disk/metadata.hpp"

namespace nmfs::structures::indexing_types::custom::on_disk {

struct metadata: public nmfs::structures::on_disk::metadata {
    uuid_t uuid;
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_ON_DISK_METADATA_HPP
