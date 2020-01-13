#ifndef NMFS_KV_BACKENDS_RADOS_BACKEND_HPP
#define NMFS_KV_BACKENDS_RADOS_BACKEND_HPP

#include "kv_backend.hpp"

namespace nmfs::kv_backends {
class rados_backend: public kv_backend {
    [[nodiscard]] owner_slice get(const slice& key) final;
    void get(const slice& key, slice& value) final;
    void put(const slice& key, const slice& value) final;
    [[nodiscard]] bool exist(const slice& key) final;
    void remove(const slice& key) final;
};
}

#endif //NMFS_KV_BACKENDS_RADOS_BACKEND_HPP
