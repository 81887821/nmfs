#ifndef NMFS_KV_BACKENDS_RADOS_BACKEND_HPP
#define NMFS_KV_BACKENDS_RADOS_BACKEND_HPP

#include <rados/librados.hpp>
#include "kv_backend.hpp"

namespace nmfs::kv_backends {

class rados_backend: public kv_backend {
public:
    struct connect_information {
        const char* user_name = "client.admin";
        const char* cluster_name = "ceph";
        const char* configuration_file = "/etc/ceph/ceph.conf";
        uint64_t flags = 0;
    };

    explicit rados_backend(const connect_information& information);
    ~rados_backend() override;

    [[nodiscard]] owner_slice get(const slice& key) final;
    ssize_t get(const slice& key, slice& value) final; // fully read
    ssize_t get(const slice& key, off_t offset, size_t length, slice& value) final; // partial read

    ssize_t put(const slice& key, const slice& value) final; // fully write
    ssize_t put(const slice& key, off_t offset, const slice& value) final; // partial write

    [[nodiscard]] bool exist(const slice& key) final;
    void remove(const slice& key) final;

private:
    static constexpr const char* pool_name = "cephfs_data";

    librados::Rados cluster;
    librados::IoCtx io_ctx;
};

}

#endif //NMFS_KV_BACKENDS_RADOS_BACKEND_HPP
