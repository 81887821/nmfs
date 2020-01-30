#ifndef NMFS_STRUCTURES_UTILS_DATA_OBJECT_KEY_HPP
#define NMFS_STRUCTURES_UTILS_DATA_OBJECT_KEY_HPP

#include <cstring>
#include "../../memory_slices/owner_slice.hpp"
#include "../../logger/log.hpp"
#include "../../logger/write_bytes.hpp"

namespace nmfs::structures::utils {

class data_object_key: public owner_slice {
public:
    const char separator = static_cast<char>(0x1C);

    inline data_object_key(const slice& base, uint32_t index);

    inline void update_index(uint32_t new_index);
    inline void increase_index();
    [[nodiscard]] constexpr size_t get_index() const;

private:
    size_t base_length;
    size_t index;

    /**
     * Convert host usable index to backend key compatible index
     * @param index Index in [0, 1 << 28)
     * @return Index that doesn't contain null byte
     */
    constexpr static uint32_t to_key_index(uint32_t index);
};

inline data_object_key::data_object_key(const slice& base, uint32_t index)
    : owner_slice(base.size() + sizeof(separator) + sizeof(index)),
      base_length(base.size()),
      index(index) {
    memcpy(memory.get(), base.data(), base_length);
    memory[base_length] = separator;
    *reinterpret_cast<uint32_t*>(&memory[base_length + 1]) = to_key_index(index);
}

inline void data_object_key::update_index(uint32_t new_index) {
    this->index = new_index;
    *reinterpret_cast<uint32_t*>(&memory[base_length + 1]) = to_key_index(index);
}

inline void data_object_key::increase_index() {
    update_index(index + 1);
}

constexpr size_t data_object_key::get_index() const {
    return index;
}

constexpr uint32_t data_object_key::to_key_index(uint32_t index) {
    if (index > (1 << 28)) {
        throw std::out_of_range("Data object key index out of range: " + std::to_string(index));
    } else {
        uint32_t result = 0;
        auto result_as_bytes = reinterpret_cast<uint8_t*>(&result);

        /**
         * 0th to 6th bits of result_as_bytes[i] is from index
         * 7th bit of result_as_bytes[i] is always 1
         */
        for (size_t i = 0; i < sizeof(index); i++) {
            const auto msb = static_cast<uint8_t>(1 << 7);
            uint8_t value = (index >> (i * 7)) & 0x7F;
            result_as_bytes[i] = msb | value;
        }

        return result;
    }
}

}

#endif //NMFS_STRUCTURES_UTILS_DATA_OBJECT_KEY_HPP
