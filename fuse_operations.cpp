#include <iostream>
#include <string>
#include <rados/librados.hpp>

#include "fuse_operations.hpp"
#include "constants.hpp"
#include "memory_slices/slice.hpp"

void* nmfs::fuse_operations::init(struct fuse_conn_info* info, struct fuse_config* config) {
    int err;

    // Initialize the cluster handle with the "ceph" cluster name and "client.admin" user
    err = cluster.init2(user_name, cluster_name, flags);
    if (err < 0) {
        std::cerr << "Couldn't initialize the cluster handle! error " << err << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Created a cluster handle." << std::endl;
    }

    // Read a Ceph configuration file to configure the cluster handle.
    err = cluster.conf_read_file("/etc/ceph/ceph.conf");
    if (err < 0) {
        std::cerr << "Couldn't read the Ceph configuration file! error " << err << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Read the Ceph configuration file." << std::endl;
    }

    // Connect to the cluster
    err = cluster.connect();
    if (err < 0) {
        std::cerr << "Couldn't connect to cluster! error " << err << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Connected to the cluster." << std::endl;
    }

    // Create an ioctx for the data pool
    err = cluster.ioctx_create(pool_name, io_ctx);
    if (err < 0) {
        std::cerr << "Couldn't set up ioctx! error " << err << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Created an ioctx for the pool." << std::endl;
    }

    ///////////////
    return NULL;
}

void nmfs::fuse_operations::destroy(void* private_data) {
    io_ctx.close();
    cluster.shutdown();

    std::cout << "Terminate nmFS successfully." << std::endl;
}
