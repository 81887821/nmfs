#ifndef NMFS_MEMORY_SLICES_OWNER_SLICE_HPP
#define NMFS_MEMORY_SLICES_OWNER_SLICE_HPP

#include <algorithm>
#include <memory>
#include "slice.hpp"

namespace nmfs {

/**
 * Slice class that owns the memory
 *
 * Memory will be allocated when owner_slice is constructed, and freed when destructed
 */
class owner_slice: public slice {
public:
    explicit inline owner_slice(size_t size);
    explicit inline owner_slice(size_t capacity, size_t size);
    explicit inline owner_slice(const slice& other);
    inline owner_slice(const owner_slice& other);
    inline owner_slice(owner_slice&& other) = default;

    [[nodiscard]] inline byte* data() final;
    [[nodiscard]] inline const byte* data() const final;

protected:
    std::unique_ptr<byte[]> memory;
};

inline owner_slice::owner_slice(size_t size): slice(size), memory(std::make_unique<byte[]>(size)) {
}

inline owner_slice::owner_slice(size_t capacity, size_t size): slice(capacity, size), memory(std::make_unique<byte[]>(capacity)) {
    if (size > capacity) {
        throw std::invalid_argument("size is larger than capacity");
    }
}

inline owner_slice::owner_slice(const slice& other): slice(other.capacity(), other.size()), memory(std::make_unique<byte[]>(other.capacity())) {
    std::copy(other.data(), other.data() + other.size(), memory.get());
}

inline owner_slice::owner_slice(const owner_slice& other): owner_slice(static_cast<const slice&>(other)) {
}

byte* owner_slice::data() {
    return memory.get();
}

const byte* owner_slice::data() const {
    return memory.get();
}

}

#endif //NMFS_OWNER_SLICE_HPP
