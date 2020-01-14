#ifndef NMFS_MEMORY_SLICES_SLICE_HPP
#define NMFS_MEMORY_SLICES_SLICE_HPP

#include <stdexcept>
#include <cstdint>
#include "../primitive_types.hpp"

namespace nmfs {

/**
 * Abstract base class for memory slice
 */
class slice {
public:
    [[nodiscard]] virtual byte* data() = 0;
    [[nodiscard]] virtual const byte* data() const = 0;
    [[nodiscard]] constexpr size_t capacity() const;
    [[nodiscard]] constexpr size_t size() const;
    constexpr void set_size(size_t new_size);

protected:
    size_t memory_capacity;
    size_t data_size;

    constexpr explicit slice(size_t size);
    constexpr slice(size_t capacity, size_t size);
};

constexpr size_t slice::capacity() const {
    return memory_capacity;
}

constexpr size_t slice::size() const {
    return data_size;
}

constexpr void slice::set_size(size_t new_size) {
    if (new_size > memory_capacity) {
        throw std::invalid_argument("new_size is larger than capacity");
    }
    data_size = new_size;
}

constexpr slice::slice(size_t size): memory_capacity(size), data_size(size) {
}

constexpr slice::slice(size_t capacity, size_t size): memory_capacity(capacity), data_size(size) {
}


}

#endif //NMFS_SLICE_HPP
