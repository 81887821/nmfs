#include <cstring>
#include "directory.hpp"

using namespace nmfs::structures;

std::unique_ptr<nmfs::byte[]> directory::serialize() const {
    on_disk_size_type number_of_files = files.size();
    std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(size);
    size_t index = 0;

    *reinterpret_cast<on_disk_size_type*>(&buffer[index]) = number_of_files;
    index += sizeof(number_of_files);

    for (const auto& file: files) {
        on_disk_size_type file_name_length = file.size();

        *reinterpret_cast<on_disk_size_type*>(&buffer[index]) = file_name_length;
        index += sizeof(file_name_length);

        memcpy(&buffer[index], file.c_str(), file_name_length);
        index += file_name_length;
    }

    return buffer;
}

void directory::parse(std::unique_ptr<nmfs::byte[]> buffer) {
    size_t index = 0;

    on_disk_size_type number_of_files = *reinterpret_cast<on_disk_size_type*>(&buffer[index]);
    index += sizeof(on_disk_size_type);

    for (on_disk_size_type i = 0; i < number_of_files; i++) {
        on_disk_size_type file_name_length = *reinterpret_cast<on_disk_size_type*>(&buffer[index]);
        index += sizeof(on_disk_size_type);

        auto file_name = std::string(&buffer[index], file_name_length);
        index += file_name_length;
    }

    size = index;
}
