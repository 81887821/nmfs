#ifndef NMFS_LOGGERS_LOG_HPP
#define NMFS_LOGGERS_LOG_HPP

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include "log_levels.hpp"
#include "log_locations.hpp"
#include "log_stream.hpp"
#include "no_log_stream.hpp"

namespace nmfs {

class log {
public:
    class configuration {
    public:
        static constexpr auto log_level = log_levels::information;
        static constexpr auto log_location = log_locations::all;
    };

#ifdef DEBUG
    using log_stream_type = log_stream;
#else
    using log_stream_type = no_log_stream;
#endif

    inline static log_stream_type information(log_locations log_location);
    inline static log_stream_type debug(log_locations log_location);
    inline static log_stream_type warning(log_locations log_location);
    inline static log_stream_type error(log_locations log_location);

    inline static log_stream_type write_log(log_levels level, log_locations location);

private:
    inline static bool logging_needed(log_levels level, log_locations location);
};

inline log::log_stream_type log::information(log_locations log_location) {
    return write_log(log_levels::information, log_location);
}

inline log::log_stream_type log::debug(log_locations log_location) {
    return write_log(log_levels::debug, log_location);
}

inline log::log_stream_type log::warning(log_locations log_location) {
    return write_log(log_levels::warning, log_location);
}

inline log::log_stream_type log::error(log_locations log_location) {
    return write_log(log_levels::error, log_location);
}

inline log::log_stream_type log::write_log(log_levels level, log_locations location) {
#ifdef DEBUG
    if (logging_needed(level, location)) {
        std::ostream& logging_backend = std::cerr;
        return log_stream(logging_backend, level, location);
    } else {
        return log_stream();
    }
#else
    return no_log_stream();
#endif
}

inline bool log::logging_needed(log_levels level, log_locations location) {
    return level >= configuration::log_level && (location & configuration::log_location);
}

}

#endif //NMFS_LOGGERS_LOG_HPP
