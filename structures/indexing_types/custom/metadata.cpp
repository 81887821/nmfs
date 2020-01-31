#include <algorithm>
#include "../../../memory_slices/borrower_slice.hpp"
#include "metadata.hpp"
#include "../../../utils.hpp"
#include "../../../kv_backends/exceptions/key_does_not_exist.hpp"

using namespace nmfs::structures::indexing_types::custom;

metadata::metadata(nmfs::structures::super_object& super, nmfs::owner_slice key, uid_t owner, gid_t group, mode_t mode)
    : nmfs::structures::metadata(super, std::move(key), owner, group, mode),
      data_key_base(generate_uuid()) {
}

metadata::metadata(nmfs::structures::super_object& super, nmfs::owner_slice key, const nmfs::structures::on_disk::metadata* on_disk_data)
    : nmfs::structures::metadata(super, std::move(key), on_disk_data),
      data_key_base(sizeof(uuid_t)) {
    auto custom_on_disk_data = static_cast<const nmfs::structures::indexing_types::custom::on_disk::metadata*>(on_disk_data);
    std::copy(custom_on_disk_data->uuid, custom_on_disk_data->uuid + sizeof(uuid_t), data_key_base.data());
}

metadata::metadata(metadata&& other, nmfs::owner_slice key)
    : nmfs::structures::metadata(std::move(other), std::move(key), other.data_key_base),
      data_key_base(std::move(other.data_key_base)) {
}

metadata::~metadata() {
    metadata::flush();
}

void metadata::flush() const {
    auto lock = std::shared_lock(*mutex);

    if (dirty) {
        nmfs::structures::indexing_types::custom::on_disk::metadata on_disk_structure {};
        to_on_disk_metadata(on_disk_structure);

        auto value = borrower_slice(&on_disk_structure, sizeof(on_disk_structure));

        context.backend->put(key, value);
        dirty = false;
    }
}

void metadata::reload() {
    // TODO
}

void metadata::move_data(const nmfs::slice& new_data_key_base) {
    if (data_key_base != new_data_key_base) {
        auto old_data_key = nmfs::structures::utils::data_object_key(data_key_base, 0);
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

        data_key_base = owner_slice(new_data_key_base.size());
        std::copy(new_data_key_base.data(), new_data_key_base.data() + new_data_key_base.size(), data_key_base.data());
    }
}

nmfs::structures::utils::data_object_key metadata::get_data_object_key(uint32_t index) const {
    return nmfs::structures::utils::data_object_key(data_key_base, index);
}

void metadata::to_on_disk_metadata(nmfs::structures::indexing_types::custom::on_disk::metadata& on_disk_metadata) const {
    nmfs::structures::metadata::to_on_disk_metadata(on_disk_metadata);
    std::copy(data_key_base.data(), data_key_base.data() + sizeof(uuid_t), on_disk_metadata.uuid);
}
