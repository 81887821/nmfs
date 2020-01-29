#ifndef NMFS_STRUCTURES_DIRECTORY_ENTRY_HPP
#define NMFS_STRUCTURES_DIRECTORY_ENTRY_HPP

#include <algorithm>
#include <string>
#include "../primitive_types.hpp"
#include "../fuse.hpp"
#include "metadata.hpp"

namespace nmfs::structures {

class directory_entry {
public:
    std::string file_name;

    inline directory_entry(std::string file_name, const metadata& metadata);
    inline explicit directory_entry(const byte** buffer);

    inline virtual on_disk_size_type serialize(byte* buffer) const;
    [[nodiscard]] inline virtual size_t size() const;
    inline virtual void fill(const fuse_directory_filler& filler) const;

    inline virtual bool operator<(const directory_entry& other) const;
    inline virtual bool operator==(const directory_entry& other) const;

    inline static std::function<bool(const directory_entry&)> find_by_name(std::string_view file_name);
};

directory_entry::directory_entry(std::string file_name, const metadata& metadata): file_name(std::move(file_name)) {
}

directory_entry::directory_entry(const byte** buffer) {
    on_disk_size_type file_name_length = *reinterpret_cast<const on_disk_size_type*>(*buffer);
    (*buffer) += sizeof(on_disk_size_type);

    file_name = std::string(*buffer, file_name_length);
    (*buffer) += file_name_length;
}

on_disk_size_type directory_entry::serialize(byte* buffer) const {
    on_disk_size_type file_name_length = file_name.length();
    *reinterpret_cast<on_disk_size_type*>(buffer) = file_name_length;

    std::copy(file_name.begin(), file_name.end(), buffer + sizeof(on_disk_size_type));

    return sizeof(on_disk_size_type) + file_name_length;
}

size_t directory_entry::size() const {
    return sizeof(on_disk_size_type) + file_name.size();
}

void directory_entry::fill(const fuse_directory_filler& filler) const {
    filler(file_name.c_str());
}

std::function<bool(const directory_entry&)> directory_entry::find_by_name(std::string_view file_name) {
    return [file_name] (const directory_entry& other) {
        return file_name == other.file_name;
    };
}

bool directory_entry::operator<(const directory_entry& other) const {
    return file_name < other.file_name;
}

bool directory_entry::operator==(const directory_entry& other) const {
    return file_name == other.file_name;
}

}

#endif //NMFS_STRUCTURES_DIRECTORY_ENTRY_HPP
