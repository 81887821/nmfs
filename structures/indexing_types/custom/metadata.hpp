#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_METADATA_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_METADATA_HPP

#include "../../metadata.hpp"
#include "on_disk/metadata.hpp"

namespace nmfs::structures::indexing_types::custom {

class metadata: public nmfs::structures::metadata {
public:
    owner_slice data_key_base;

    metadata(super_object& super, owner_slice key, uid_t owner, gid_t group, mode_t mode);
    metadata(super_object& super, owner_slice key, const nmfs::structures::on_disk::metadata* on_disk_data);
    metadata(metadata&& other, owner_slice key);
    ~metadata() override;

    void flush() const override;
    void reload() override;
    void move_data(const slice& new_data_key_base) override;

protected:
    utils::data_object_key get_data_object_key(uint32_t index) const override;
    virtual void to_on_disk_metadata(nmfs::structures::indexing_types::custom::on_disk::metadata& on_disk_metadata) const;
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_METADATA_HPP
