#ifndef NMFS_RADOS_BACKEND_HPP
#define NMFS_RADOS_BACKEND_HPP

#include "kv_backend.hpp"

namespace nmfs::kv_backends {
class rados_backend: public kv_backend {
    [[nodiscard]] owner_slice get(const slice& key) final;

    void get(const slice& key, slice& value) final; // fully read
    void get(const slice& key, off_t offset, size_t length, slice& value); // partial read

    void put(const slice& key, const slice& value) final; // fully write
    void put(const slice& key, off_t offset, size_t length, const slice& value); // partial write

    [[nodiscard]] bool exist(const slice& key) final;
    void remove(const slice& key) final;
};
}

#endif //NMFS_RADOS_BACKEND_HPP
