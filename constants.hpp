#ifndef NMFS_CONSTANTS_HPP
#define NMFS_CONSTANTS_HPP

#include <rados/librados.hpp>

namespace nmfs {

// librados handler variables
static librados::Rados cluster;
const char cluster_name[] = "ceph";
const char user_name[] = "client.admin";
static uint64_t flags;

//librados io context
static librados::IoCtx io_ctx;
const char pool_name[] = "cephfs_data";

}

#endif
