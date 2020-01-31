#ifndef NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_METADATA_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_METADATA_HPP

#include "../../metadata.hpp"

namespace nmfs::structures::indexing_types::full_path {

class metadata: public nmfs::structures::metadata {
public:
    metadata(super_object& super, owner_slice key, uid_t owner, gid_t group, mode_t mode);
    metadata(super_object& super, owner_slice key, const on_disk::metadata* on_disk_data);
    metadata(metadata&& other, owner_slice key);
    ~metadata() override;

    void flush() const override;
    void reload() override;
    void move_data(const slice& new_data_key_base) override;

protected:
    utils::data_object_key get_data_object_key(uint32_t index) const override;
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_METADATA_HPP
