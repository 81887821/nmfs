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

std::string nmfs::generate_uuid() {
    char uuid[37];
    uuid_t generated_uuid;

    uuid_generate_random(generated_uuid);
    uuid_unparse(generated_uuid, uuid);

    return std::string(uuid);
}
