#include "utils.hpp"
#include "memory_slices/borrower_slice.hpp"

std::string_view nmfs::get_parent_directory(std::string_view path) {
    if (path.empty()) {
        throw std::runtime_error("Invalid path: path is empty string");
    } else {
        for (size_t i = path.length() - 1; i > 0; i--) {
            if (path[i] == path_delimiter) {
                return path.substr(0, i);
            }
        }

        if (path[0] == path_delimiter) {
            return path.substr(0, 1); // root directory
        }

        throw std::runtime_error("Invalid path: No delimiter on path");
    }
}

std::string_view nmfs::get_filename(std::string_view path) {
    if (path.empty()) {
        throw std::runtime_error("Invalid path: path is empty string");
    } else {
        for (size_t i = path.length() - 1; i > 0; i--) {
            if (path[i] == path_delimiter) {
                return path.substr(i + 1);
            }
        }

        if (path[0] == path_delimiter) {
            return path.substr(1); // root directory
        }

        throw std::runtime_error("Invalid path: No delimiter on path");
    }
}

nmfs::owner_slice nmfs::generate_uuid() {
    auto uuid = owner_slice(sizeof(uuid_t));

    uuid_generate_random(reinterpret_cast<uint8_t*>(uuid.data()));

    return uuid;
}
