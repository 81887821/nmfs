#include <iostream>
#include <cstring>
#include <stdexcept>
#include "rados_backend.hpp"

nmfs::kv_backends::rados_backend::rados_backend(const nmfs::kv_backends::rados_backend::connect_information& information) {
    int err;

    // Initialize the cluster handle with the "ceph" cluster name and "client.admin" user
    err = cluster.init2(information.user_name, information.cluster_name, information.flags);
    if (err < 0) {
        std::cerr << "Couldn't initialize the cluster handle! error " << err << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Created a cluster handle." << std::endl;
    }

    // Read a Ceph configuration file to configure the cluster handle.
    err = cluster.conf_read_file(information.configuration_file);
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
}

nmfs::kv_backends::rados_backend::~rados_backend() {
    io_ctx.close();
    cluster.shutdown();
}

nmfs::owner_slice nmfs::kv_backends::rados_backend::get(const nmfs::slice& key) {
    uint64_t object_size;
    time_t object_mtime;
    int ret;

    ret = io_ctx.stat(key.data(), &object_size, &object_mtime); // 0 on success, negative error code on failure
    if (ret < 0) {
        std::cerr << "rados_backend::get : Cannot perform read from object on " << key.data() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "rados_backend::get : Successfully write the object on " << key.data() << std::endl;
    }

    auto slice = owner_slice(object_size);
    auto buffer_list = librados::bufferlist::static_from_mem(slice.data(), slice.capacity());

    ret = io_ctx.read(key.data(), buffer_list, object_size, 0);
    if (ret < 0) {
        std::cerr << "rados_backend::get : Cannot perform partial read from object on " << key.data() << std::endl;
        throw std::runtime_error("rados_backend::get : No such file or directory");
    } else {
        std::cout << "rados_backend::get : Successfully write the object on " << key.data() << std::endl;
    }

    slice.set_size(ret);
    return slice;
}

ssize_t nmfs::kv_backends::rados_backend::get(const nmfs::slice& key, nmfs::slice& value) { // fully read
    uint64_t object_size;
    time_t object_mtime;
    librados::bufferlist buffer_list = librados::bufferlist::static_from_mem(value.data(), value.capacity());
    int ret;

    ret = io_ctx.stat(key.data(), &object_size, &object_mtime); // 0 on success, negative error code on failure
    if (ret < 0) {
        std::cerr << "rados_backend::get : Cannot perform read from object on " << key.data() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "rados_backend::get : Successfully write the object on " << key.data() << std::endl;
    }

    if (object_size > value.capacity()) {
        throw std::out_of_range("rados_backend::get : capacity of value slice is not enough");
    }

    ret = io_ctx.read(key.data(), buffer_list, value.capacity(), 0); // number of bytes read on success, negative error code on failure
    if (ret < 0) {
        std::cerr << "rados_backend::get : Cannot perform partial read from object on " << key.data() << std::endl;
        throw std::runtime_error("rados_backend::get : No such file or directory");
    } else {
        std::cout << "rados_backend::get : Successfully write the object on " << key.data() << std::endl;
    }

    value.set_size(ret);
    return ret;
}

ssize_t nmfs::kv_backends::rados_backend::get(const nmfs::slice& key, off_t offset, size_t length, nmfs::slice& value) { // partial read
    librados::bufferlist buffer_list = librados::bufferlist::static_from_mem(value.data(), value.capacity());
    int ret;

    if (length > value.capacity()) {
        throw std::out_of_range("rados_backend::get : returned object size exceeds capacity of value slice");
    }

    ret = io_ctx.read(key.data(), buffer_list, length, offset); // number of bytes read on success, negative error code on failure
    if (ret < 0) {
        std::cerr << "rados_backend::get : Cannot perform partial read from object on " << key.data() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "rados_backend::get : Successfully write the object on " << key.data() << std::endl;
    }

    value.set_size(ret);
    return ret;
}
ssize_t nmfs::kv_backends::rados_backend::put(const nmfs::slice& key, const nmfs::slice& value) { // fully write
    librados::bufferlist write_buffer = librados::bufferlist::static_from_mem(const_cast<char*>(value.data()), value.size());
    int ret;

    ret = io_ctx.write_full(key.data(), write_buffer); // 0 on success, negative error code on failure
    if(ret < 0){
        std::cerr << "rados_backend::put : Cannot perform fully write in object on " << key.data() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "rados_backend::put : Successfully write the object on " << key.data() << std::endl;
    }

    return ret;
}

ssize_t nmfs::kv_backends::rados_backend::put(const nmfs::slice& key, off_t offset, const nmfs::slice& value) { // partial write
    librados::bufferlist buffer_list = librados::bufferlist::static_from_mem(const_cast<char*>(value.data()), value.size());
    int ret;

    ret = io_ctx.write(key.data(), buffer_list, value.size(), offset);
    if(ret < 0){
        std::cerr << "rados_backend::put : Cannot perform partial write in object on "<< key.data() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "rados_backend::put : Successfully write the object on" << key.data() << "offset : " << offset << std::endl;
    }

    return ret;
}

bool nmfs::kv_backends::rados_backend::exist(const nmfs::slice& key) {
    uint64_t object_size;
    time_t object_mtime;
    int ret;

    ret = io_ctx.stat(key.data(), &object_size, &object_mtime); // 0 on success, negative error code on failure
    return ret == 0;
}

void nmfs::kv_backends::rados_backend::remove(const nmfs::slice& key) {
    int ret;

    ret = io_ctx.remove(key.data());
    if(ret < 0){
        std::cerr << "rados_backend::remove : Cannot perform removing object on "<< key.data() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "rados_backend::remove : Successfully remove the object on" << key.data() << "offset : " << std::endl;
    }
}
