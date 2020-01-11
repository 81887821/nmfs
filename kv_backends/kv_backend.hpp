#ifndef NMFS_KV_BACKEND_HPP
#define NMFS_KV_BACKEND_HPP

#include "../memory_slices/slice.hpp"
#include "../memory_slices/owner_slice.hpp"

namespace nmfs::kv_backends {
class kv_backend {
public:
    [[nodiscard]] virtual owner_slice get(const slice& key) = 0;
    virtual void get(const slice& key, slice& value) = 0;
    virtual void put(const slice& key, const slice& value) = 0;
    [[nodiscard]] virtual bool exist(const slice& key) = 0;
    virtual void remove(const slice& key) = 0;
};
}

#endif //NMFS_KV_BACKEND_HPP
