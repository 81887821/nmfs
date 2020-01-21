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

    all = other | kv_backend_operation | fuse_operation | directory_operation | file_data_operation,
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
        default:
            return "combined_locations";
    }
}

}

#endif //NMFS_LOGGER_LOG_LOCATIONS_HPP
