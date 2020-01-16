#ifndef NMFS_STRUCTURES_UTILS_DATA_OBJECT_KEY_HPP
#define NMFS_STRUCTURES_UTILS_DATA_OBJECT_KEY_HPP

#include <cstring>
#include "../../memory_slices/owner_slice.hpp"

namespace nmfs::structures::utils {

class data_object_key: public owner_slice {
public:
    const char separator = '/';

    inline data_object_key(const slice& base, uint32_t index);

    inline void update_index(uint32_t new_index);
    inline void increase_index();
    [[nodiscard]] constexpr size_t get_index() const;

private:
    size_t base_length;
    size_t index;
};

inline data_object_key::data_object_key(const slice& base, uint32_t index)
    : owner_slice(base.size() + sizeof(separator) + sizeof(index)),
      base_length(base.size()),
      index(index) {
    memcpy(memory.get(), base.data(), base_length);
    memory[base_length] = separator;
    *reinterpret_cast<uint32_t*>(&memory[base_length + 1]) = index;
}

inline void data_object_key::update_index(uint32_t new_index) {
    this->index = new_index;
    *reinterpret_cast<uint32_t*>(&memory[base_length + 1]) = new_index;
}

inline void data_object_key::increase_index() {
    update_index(index + 1);
}

constexpr size_t data_object_key::get_index() const {
    return index;
}

}

#endif //NMFS_STRUCTURES_UTILS_DATA_OBJECT_KEY_HPP
