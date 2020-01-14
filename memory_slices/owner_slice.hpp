#ifndef NMFS_MEMORY_SLICES_OWNER_SLICE_HPP
#define NMFS_MEMORY_SLICES_OWNER_SLICE_HPP

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

byte* owner_slice::data() {
    return memory.get();
}

const byte* owner_slice::data() const {
    return memory.get();
}

}

#endif //NMFS_OWNER_SLICE_HPP
