#include "utils.hpp"
#include "memory_slices/borrower_slice.hpp"

std::string nmfs::get_parent_directory(const std::string& path) {
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

std::string nmfs::get_filename(const std::string& path) {
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

std::unique_ptr<nmfs::slice> make_key(const char* path, uint8_t key_mode) {
    if(key_mode == 1){
        uint32_t path_length = strlen(path);
        std::unique_ptr<nmfs::borrower_slice> key;
        key = std::make_unique<nmfs::borrower_slice>((void *)path, path_length);

        return key;
    } else if(key_mode == 2){
        std::unique_ptr<nmfs::owner_slice> key;
    } else if(key_mode == 3) {
        std::unique_ptr<nmfs::owner_slice> key;
    } else {
        return nullptr;
    }
}

std::string nmfs::generate_uuid(){
    char uuid[37];
    uuid_t generated_uuid;

    uuid_generate_random(generated_uuid);
    uuid_unparse(generated_uuid, uuid);

    return std::string(uuid);
}