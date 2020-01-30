#include "metadata.hpp"
#include "../../../memory_slices/borrower_slice.hpp"

using namespace nmfs::structures::indexing_types::full_path;

metadata::metadata(nmfs::structures::super_object& super, nmfs::owner_slice key, uid_t owner, gid_t group, mode_t mode)
    : nmfs::structures::metadata(super, std::move(key), owner, group, mode) {
}

metadata::metadata(nmfs::structures::super_object& super, nmfs::owner_slice key, const on_disk::metadata* on_disk_data)
    : nmfs::structures::metadata(super, std::move(key), on_disk_data) {
}

metadata::~metadata() {
    metadata::flush();
}

void metadata::flush() const {
    auto lock = std::shared_lock(*mutex);

    if (dirty) {
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

nmfs::structures::utils::data_object_key metadata::get_data_object_key(uint32_t index) const {
    return nmfs::structures::utils::data_object_key(key, index);
}
