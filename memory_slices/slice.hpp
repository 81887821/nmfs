#ifndef NMFS_MEMORY_SLICES_SLICE_HPP
#define NMFS_MEMORY_SLICES_SLICE_HPP

#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include "../primitive_types.hpp"

namespace nmfs {

/**
 * Abstract base class for memory slice
 */
class slice {
public:
    using iterator = byte*;
    using const_iterator = const byte*;

    [[nodiscard]] virtual byte* data() noexcept = 0;
    [[nodiscard]] virtual const byte* data() const noexcept = 0;
    [[nodiscard]] constexpr size_t capacity() const;
    [[nodiscard]] constexpr size_t size() const;
    constexpr void set_size(size_t new_size);
    [[nodiscard]] inline std::string to_string() const;
    [[nodiscard]] constexpr std::string_view to_string_view() const;

    [[nodiscard]] constexpr iterator begin() noexcept;
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept;
    [[nodiscard]] constexpr iterator end() noexcept;
    [[nodiscard]] constexpr const_iterator cend() const noexcept;

    [[nodiscard]] inline bool operator==(const slice& other) const;
    [[nodiscard]] inline bool operator!=(const slice& other) const;

protected:
    size_t memory_capacity;
    size_t data_size;

    constexpr explicit slice(size_t size);
    constexpr slice(size_t capacity, size_t size);
    slice(const slice&) = default;
    constexpr slice(slice&& other) noexcept;

    constexpr slice& operator=(const slice&) = default;
    constexpr slice& operator=(slice&&) noexcept;
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

std::string slice::to_string() const {
    return std::string(data(), data_size);
}

constexpr std::string_view slice::to_string_view() const {
    return std::string_view(data(), data_size);
}

constexpr slice::iterator slice::begin() noexcept {
    return data();
}

constexpr slice::const_iterator slice::cbegin() const noexcept {
    return data();
}

constexpr slice::iterator slice::end() noexcept {
    return data() + data_size;
}

constexpr slice::const_iterator slice::cend() const noexcept {
    return data() + data_size;
}

bool slice::operator==(const slice& other) const {
    return std::equal(cbegin(), cend(), other.cbegin(), other.cend());
}

bool slice::operator!=(const slice& other) const {
    return !operator==(other);
}

constexpr slice::slice(slice&& other) noexcept
    : memory_capacity(other.memory_capacity),
      data_size(other.data_size) {
    other.memory_capacity = 0;
    other.data_size = 0;
}

constexpr slice& slice::operator=(slice&& other) noexcept {
    memory_capacity = other.memory_capacity;
    data_size = other.data_size;

    other.memory_capacity = 0;
    other.data_size = 0;

    return *this;
}

}

#endif //NMFS_SLICE_HPP
