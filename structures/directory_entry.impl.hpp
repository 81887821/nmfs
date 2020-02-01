#ifndef NMFS_STRUCTURES_DIRECTORY_ENTRY_IMPL_HPP
#define NMFS_STRUCTURES_DIRECTORY_ENTRY_IMPL_HPP

#include "directory_entry.hpp"

namespace nmfs::structures {

template<typename indexing>
directory_entry<indexing>::directory_entry(std::string file_name, const metadata<indexing>& metadata): file_name(std::move(file_name)) {
}

template<typename indexing>
directory_entry<indexing>::directory_entry(const byte** buffer) {
    on_disk_size_type file_name_length = *reinterpret_cast<const on_disk_size_type*>(*buffer);
    (*buffer) += sizeof(on_disk_size_type);

    file_name = std::string(*buffer, file_name_length);
    (*buffer) += file_name_length;
}

template<typename indexing>
on_disk_size_type directory_entry<indexing>::serialize(byte* buffer) const {
    on_disk_size_type file_name_length = file_name.length();
    *reinterpret_cast<on_disk_size_type*>(buffer) = file_name_length;

    std::copy(file_name.begin(), file_name.end(), buffer + sizeof(on_disk_size_type));

    return sizeof(on_disk_size_type) + file_name_length;
}

template<typename indexing>
size_t directory_entry<indexing>::size() const {
    return sizeof(on_disk_size_type) + file_name.size();
}

template<typename indexing>
void directory_entry<indexing>::fill(const fuse_directory_filler& filler) const {
    filler(file_name.c_str());
}

template<typename indexing>
std::function<bool(const directory_entry<indexing>&)> directory_entry<indexing>::find_by_name(std::string_view file_name) {
    return [file_name] (const directory_entry& other) {
        return file_name == other.file_name;
    };
}

template<typename indexing>
bool directory_entry<indexing>::operator<(const directory_entry& other) const {
    return file_name < other.file_name;
}

template<typename indexing>
bool directory_entry<indexing>::operator==(const directory_entry& other) const {
    return file_name == other.file_name;
}

}

#endif //NMFS_STRUCTURES_DIRECTORY_ENTRY_IMPL_HPP
