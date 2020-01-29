#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_HPP

#include <algorithm>
#include "../../directory_entry.hpp"
#include "metadata.hpp"

namespace nmfs::structures::indexing_types::custom {

class directory_entry: public nmfs::structures::directory_entry {
public:
    uuid_t uuid{};

    inline directory_entry(std::string file_name, const nmfs::structures::metadata& metadata);
    inline explicit directory_entry(const byte** buffer);

    inline on_disk_size_type serialize(byte* buffer) const override;
    [[nodiscard]] inline size_t size() const override;
};

directory_entry::directory_entry(std::string file_name, const nmfs::structures::metadata& metadata): nmfs::structures::directory_entry(std::move(file_name), metadata) {
    auto custom_metadata = dynamic_cast<const nmfs::structures::indexing_types::custom::metadata&>(metadata);

    std::copy(custom_metadata.data_key_base.data(), custom_metadata.data_key_base.data() + sizeof(uuid_t), uuid);
}

directory_entry::directory_entry(const byte** buffer): nmfs::structures::directory_entry(buffer) {
    std::copy(*buffer, (*buffer) + sizeof(uuid_t), uuid);
    (*buffer) += sizeof(uuid_t);
}

on_disk_size_type directory_entry::serialize(byte* buffer) const {
    byte* current = buffer + nmfs::structures::directory_entry::serialize(buffer);

    std::copy(uuid, uuid + sizeof(uuid), current);
    current += sizeof(uuid);

    return current - buffer;
}

size_t directory_entry::size() const {
    return nmfs::structures::directory_entry::size() + sizeof(uuid_t);
}

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_HPP
