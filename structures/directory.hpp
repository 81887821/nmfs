#ifndef NMFS_STRUCTURES_DIRECTORY_HPP
#define NMFS_STRUCTURES_DIRECTORY_HPP

#include <memory>
#include <string>
#include <set>
#include "metadata.hpp"

namespace nmfs::structures {

class directory {
    using on_disk_size_type = uint32_t;

public:
    explicit inline directory(metadata& metadata);

    inline void add_file(std::string file_name);
    inline void remove_file(const std::string& file_name);
    inline void sync() const;

private:
    metadata& directory_metadata;
    std::set<std::string> files;
    size_t size;

    [[nodiscard]] std::unique_ptr<byte[]> serialize() const;
    void parse(std::unique_ptr<byte[]> buffer);
};

inline directory::directory(nmfs::structures::metadata& metadata): directory_metadata(metadata), size(sizeof(on_disk_size_type)) {
    if (metadata.size > 0) {
        std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(metadata.size);

        metadata.read(buffer.get(), metadata.size, 0);
        parse(std::move(buffer));
    }
}

inline void directory::add_file(std::string file_name) {
    size_t file_name_length = file_name.length();

    files.emplace(std::move(file_name));
    size += sizeof(on_disk_size_type) + file_name_length;
}

inline void directory::remove_file(const std::string& file_name) {
    if (files.erase(file_name)) {
        size -= sizeof(on_disk_size_type) + file_name.length();
    }
}

inline void directory::sync() const {
    if (size < directory_metadata.size) {
        directory_metadata.truncate(size);
    }
    directory_metadata.write(serialize().get(), size, 0);
}

}

#endif //NMFS_STRUCTURES_DIRECTORY_HPP
