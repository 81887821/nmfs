#ifndef NMFS_LOGGER_NO_LOG_STREAM_HPP
#define NMFS_LOGGER_NO_LOG_STREAM_HPP

namespace nmfs {

class no_log_stream {
public:
    template<typename type>
    constexpr no_log_stream& operator<<(const type& t) {
        return *this;
    }
};

}

#endif //NMFS_LOGGER_NO_LOG_STREAM_HPP
