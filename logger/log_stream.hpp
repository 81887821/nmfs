#ifndef NMFS_LOGGER_LOG_STREAM_HPP
#define NMFS_LOGGER_LOG_STREAM_HPP

#include <sstream>
#include <iostream>
#include <string>
#include <optional>
#include <functional>
#include "log_levels.hpp"
#include "log_locations.hpp"

namespace nmfs {

class log_stream: public std::stringstream {
public:
    explicit inline log_stream();
    explicit inline log_stream(std::ostream& logging_backend, log_levels level, log_locations location);
    inline ~log_stream() override;

private:
    std::optional<std::reference_wrapper<std::ostream>> logging_backend;
};

log_stream::log_stream(): logging_backend(std::nullopt) {
}

log_stream::log_stream(std::ostream& logging_backend, log_levels level, log_locations location)
    : logging_backend(logging_backend) {
    *this << "[" << to_string(level) << "][" << to_string(location) << "] ";
}

log_stream::~log_stream() {
    if (logging_backend) {
        logging_backend->get() << rdbuf();
    }
}

}

#endif //NMFS_LOGGER_LOG_STREAM_HPP
