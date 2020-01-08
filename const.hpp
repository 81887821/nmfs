#ifndef CONST_H
#define CONST_H

#include <rados/librados.hpp>

// librados handler variables 
librados::Rados cluster;
const char cluster_name[] = "ceph";
const char user_name[] = "client.admin";
uint64_t flags;

//librados io context
librados::IoCtx io_ctx;
const char *pool_name = "cephfs_data";

#endif
