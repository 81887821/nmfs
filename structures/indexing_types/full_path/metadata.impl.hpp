#ifndef NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_METADATA_IMPL_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_METADATA_IMPL_HPP

#include "metadata.hpp"
#include "../../../memory_slices/borrower_slice.hpp"
#include "../../../kv_backends/exceptions/key_does_not_exist.hpp"

#include "../../metadata.impl.hpp"

namespace nmfs::structures::indexing_types::full_path {

metadata::metadata(nmfs::structures::super_object<indexing>& super, nmfs::owner_slice key, uid_t owner, gid_t group, mode_t mode)
    : nmfs::structures::metadata<indexing>(super, std::move(key), owner, group, mode) {
}

metadata::metadata(nmfs::structures::super_object<indexing>& super, nmfs::owner_slice key, const on_disk::metadata* on_disk_data)
    : nmfs::structures::metadata<indexing>(super, std::move(key), on_disk_data) {
}
metadata::metadata(metadata&& other, nmfs::owner_slice key)
    : nmfs::structures::metadata<indexing>(std::move(other), std::move(key)) {
}


metadata::~metadata() {
    metadata::flush();
}

void metadata::flush() const {
    if (valid && dirty) {
        on_disk::metadata on_disk_structure {};
        to_on_disk_metadata(on_disk_structure);

        auto value = borrower_slice(&on_disk_structure, sizeof(on_disk_structure));

        context.backend->put(key, value);
        dirty = false;
    }
}

void metadata::reload() {
    // TODO
}

void metadata::move_data(const slice& new_data_key_base) {
    auto old_data_key = nmfs::structures::utils::data_object_key(key, 0);
    auto new_data_key = nmfs::structures::utils::data_object_key(new_data_key_base, 0);

    for (size_t i = 0; i < size / context.maximum_object_size; i++) {
        try {
            owner_slice data = context.backend->get(old_data_key);
            context.backend->remove(old_data_key);
            context.backend->put(new_data_key, data);
        } catch (nmfs::kv_backends::exceptions::key_does_not_exist&) {
            continue;
        }
    }
}

nmfs::structures::utils::data_object_key metadata::get_data_object_key(uint32_t index) const {
    return nmfs::structures::utils::data_object_key(key, index);
}

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_FULL_PATH_METADATA_IMPL_HPP
