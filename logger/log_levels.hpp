#ifndef NMFS_LOGGER_LOG_LEVELS_HPP
#define NMFS_LOGGER_LOG_LEVELS_HPP

#include <stdexcept>
#include <string>

namespace nmfs {

enum log_levels {
    information,
    debug,
    warning,
    error,
};

constexpr const char* to_string(log_levels level) {
    switch (level) {
        case information:
            return "information";
        case debug:
            return "debug";
        case warning:
            return "warning";
        case error:
            return "error";
        default:
            throw std::out_of_range("Invalid log level" + std::to_string(level));
    }
}

}

#endif //NMFS_LOGGER_LOG_LEVELS_HPP
