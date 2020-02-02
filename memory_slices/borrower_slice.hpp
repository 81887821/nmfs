#ifndef NMFS_MEMORY_SLICES_BORROWER_SLICE_HPP
#define NMFS_MEMORY_SLICES_BORROWER_SLICE_HPP

#include <string>
#include <vector>
#include <array>
#include "slice.hpp"

#define DECLARE_CONST_BORROWER_SLICE(instance_name, memory, size) const nmfs::borrower_slice instance_name(const_cast<void*>(static_cast<const void*>(memory)), size)

namespace nmfs {

/**
 * Slice class that borrows memory
 *
 * Underlying memory should remain allocated while borrower_slice is used
 */
class borrower_slice: public slice {
public:
    constexpr borrower_slice(void* memory, size_t size);
    template<typename char_type>
    constexpr borrower_slice(std::basic_string<char_type>& string);
    template<typename type>
    constexpr borrower_slice(std::vector<type>& vector);
    template<typename type, size_t array_size>
    constexpr borrower_slice(std::array<type, array_size>& array);
    borrower_slice(const borrower_slice&) = default;
    borrower_slice(borrower_slice&&) noexcept = default;

    [[nodiscard]] inline byte* data() noexcept final;
    [[nodiscard]] inline const byte* data() const noexcept final;

protected:
    void* memory;
};

constexpr borrower_slice::borrower_slice(void* memory, size_t size): slice(size, size), memory(memory) {
}

template<typename char_type>
constexpr borrower_slice::borrower_slice(std::basic_string<char_type>& string): slice(string.capacity() * sizeof(char_type), string.size() * sizeof(char_type)), memory(const_cast<char_type*>(string.c_str())) {
}

template<typename type>
constexpr borrower_slice::borrower_slice(std::vector<type>& vector): slice(vector.capacity() * sizeof(type), vector.size() * sizeof(type)), memory(vector.data()) {
}

template<typename type, size_t array_size>
constexpr borrower_slice::borrower_slice(std::array<type, array_size>& array): slice(array_size * sizeof(type)), memory(array.data()) {
}

byte* borrower_slice::data() noexcept {
    return static_cast<byte*>(memory);
}

const byte* borrower_slice::data() const noexcept {
    return static_cast<const byte*>(memory);
}

}

#endif //NMFS_BORROWER_SLICE_HPP
