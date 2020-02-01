#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_IMPL_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_IMPL_HPP

#include <algorithm>
#include "directory_entry.hpp"

#include "../../directory_entry.impl.hpp"
#include "metadata.impl.hpp"

namespace nmfs::structures::indexing_types::custom {

directory_entry::directory_entry(std::string file_name, const nmfs::structures::metadata<indexing>& metadata)
    : nmfs::structures::directory_entry<indexing>(std::move(file_name), metadata), type(metadata.mode & S_IFMT) {
    auto& custom_metadata = dynamic_cast<const nmfs::structures::indexing_types::custom::metadata&>(metadata);

    std::copy(custom_metadata.data_key_base.data(), custom_metadata.data_key_base.data() + sizeof(uuid_t), uuid);
}

directory_entry::directory_entry(const byte** buffer): nmfs::structures::directory_entry<indexing>(buffer) {
    std::copy(*buffer, (*buffer) + sizeof(uuid_t), uuid);
    (*buffer) += sizeof(uuid_t);

    type = *reinterpret_cast<const uint32_t*>(*buffer);
    (*buffer) += sizeof(uint32_t);
}

on_disk_size_type directory_entry::serialize(byte* buffer) const {
    byte* current = buffer + nmfs::structures::directory_entry<indexing>::serialize(buffer);

    std::copy(uuid, uuid + sizeof(uuid), current);
    current += sizeof(uuid);

    *reinterpret_cast<uint32_t*>(current) = type;
    current += sizeof(uint32_t);

    return current - buffer;
}

size_t directory_entry::size() const {
    return nmfs::structures::directory_entry<indexing>::size() + sizeof(uuid_t) + sizeof(uint32_t);
}

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_IMPL_HPP
