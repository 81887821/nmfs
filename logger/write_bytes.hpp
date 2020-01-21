#ifndef NMFS_LOGGER_WRITE_BYTES_HPP
#define NMFS_LOGGER_WRITE_BYTES_HPP

#include <ostream>
#include <iomanip>
#include "../primitive_types.hpp"

namespace nmfs {


struct write_bytes {
    const byte* value;
    size_t size;

    constexpr write_bytes(const byte* data, size_t size);
};

constexpr write_bytes::write_bytes(const byte* data, size_t size): value(data), size(size) {
}

template<typename char_type, typename traits_type>
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& stream, write_bytes data) {
    if (data.size > 0) {
        auto original_flags = stream.flags();
        auto original_fill = stream.fill('0');
        auto original_width = stream.width();

        stream << std::hex << std::noshowbase << std::setw(2) << static_cast<unsigned>(static_cast<uint8_t>(data.value[0]));
        for (size_t i = 1; i < data.size; i++) {
            stream << ' ' << std::setw(2) << static_cast<unsigned>(static_cast<uint8_t>(data.value[i]));
        }

        stream.width(original_width);
        stream.fill(original_fill);
        stream.flags(original_flags);
    }

    return stream;
}

}

#endif //NMFS_LOGGER_WRITE_BYTES_HPP
