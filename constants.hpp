#ifndef NMFS_CONSTANTS_HPP
#define NMFS_CONSTANTS_HPP

#include <rados/librados.hpp>

namespace nmfs {

// librados handler variables
librados::Rados cluster;
const char cluster_name[] = "ceph";
const char user_name[] = "client.admin";
uint64_t flags;

//librados io context
librados::IoCtx io_ctx;
const char* pool_name = "cephfs_data";

}

#endif
