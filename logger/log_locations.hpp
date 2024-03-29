#ifndef NMFS_LOGGER_LOG_LOCATIONS_HPP
#define NMFS_LOGGER_LOG_LOCATIONS_HPP

#include <cstdint>

namespace nmfs {

enum log_locations: uint32_t {
    other = 0x1,
    kv_backend_operation = 0x2,
    fuse_operation = 0x4,
    directory_operation = 0x8,
    file_data_operation = 0x10,
    file_data_content = 0x20,
    cache_store_operation = 0x40,

    all_except_content = other | kv_backend_operation | fuse_operation | directory_operation | file_data_operation | cache_store_operation,
    all = all_except_content | file_data_content,
};

constexpr const char* to_string(log_locations location) {
    switch (location) {
        case other:
            return "other";
        case kv_backend_operation:
            return "kv_backend";
        case fuse_operation:
            return "fuse";
        case directory_operation:
            return "directory";
        case file_data_operation:
            return "file_data";
        case file_data_content:
            return "file_data_content";
        case cache_store_operation:
            return "cache_store";
        default:
            return "combined_locations";
    }
}

}

#endif //NMFS_LOGGER_LOG_LOCATIONS_HPP
