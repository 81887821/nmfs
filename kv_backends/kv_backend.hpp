#ifndef NMFS_KV_BACKENDS_KV_BACKEND_HPP
#define NMFS_KV_BACKENDS_KV_BACKEND_HPP

#include "../memory_slices/slice.hpp"
#include "../memory_slices/owner_slice.hpp"

namespace nmfs::kv_backends {

class kv_backend {
public:
    virtual ~kv_backend() = default;

    [[nodiscard]] virtual owner_slice get(const slice& key) = 0;
    [[nodiscard]] virtual owner_slice get(const slice& key, size_t length, off_t offset = 0) = 0;
    virtual ssize_t get(const slice& key, slice& value) = 0;
    virtual ssize_t get(const slice& key, off_t offset, size_t length, slice& value) = 0;

    virtual ssize_t put(const slice& key, const slice& value) = 0;
    virtual ssize_t put(const slice& key, off_t offset, const slice& value) = 0;

    [[nodiscard]] virtual bool exist(const slice& key) = 0;
    virtual void remove(const slice& key) = 0;
};

}

#endif //NMFS_KV_BACKENDS_KV_BACKEND_HPP
