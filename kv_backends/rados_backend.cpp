#include <iostream>
#include <cstring>
#include <stdexcept>
#include "rados_backend.hpp"
#include "exceptions/backend_initialization_failure.hpp"
#include "exceptions/key_does_not_exist.hpp"
#include "../logger/log.hpp"
#include "../logger/write_bytes.hpp"

using namespace nmfs::kv_backends::exceptions;

nmfs::kv_backends::rados_backend::rados_backend(const nmfs::kv_backends::rados_backend::connect_information& information) {
    int err;

    // Initialize the cluster handle with the "ceph" cluster name and "client.admin" user
    err = cluster.init2(information.user_name, information.cluster_name, information.flags);
    if (err < 0) {
        throw backend_initialization_failure("Couldn't initialize the cluster handle", err);
    } else {
        log::information(log_locations::kv_backend_operation)
            << "Created a cluster handle.\n";
    }

    // Read a Ceph configuration file to configure the cluster handle.
    err = cluster.conf_read_file(information.configuration_file);
    if (err < 0) {
        throw backend_initialization_failure("Couldn't read the ceph configuration file", err);
    } else {
        log::information(log_locations::kv_backend_operation)
            << "Read the Ceph configuration file.\n";
    }

    // Connect to the cluster
    err = cluster.connect();
    if (err < 0) {
        throw backend_initialization_failure("Couldn't connect to the cluster", err);
    } else {
        log::information(log_locations::kv_backend_operation)
            << "Connected to the cluster.\n";
    }

    // Create an ioctx for the data pool
    err = cluster.ioctx_create(pool_name, io_ctx);
    if (err < 0) {
        throw backend_initialization_failure("Couldn't set up io context", err);
    } else {
        log::information(log_locations::kv_backend_operation)
            << "Created an ioctx for the pool.\n";
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

    ret = io_ctx.stat(key.to_string(), &object_size, &object_mtime); // 0 on success, negative error code on failure
    if (ret >= 0) {
        log::information(log_locations::kv_backend_operation)
            << "rados_backend::get : stat(key = " << key.to_string_view() << ") = " << ret << "\n";
    } else if (ret == -ENOENT) {
        throw key_does_not_exist(key);
    } else {
        throw generic_kv_api_failure("rados_backend::get : stat failed (key = " + key.to_string() + ')', ret);
    }

    auto slice = owner_slice(object_size);
    auto buffer_list = librados::bufferlist::static_from_mem(slice.data(), slice.capacity());

    ret = io_ctx.read(key.to_string(), buffer_list, object_size, 0);
    if (ret >= 0) {
        log::information(log_locations::kv_backend_operation)
            << "rados_backend::get : full read(key = " << key.to_string_view() << ") = " << ret << "\n";
    } else if (ret == -ENOENT) {
        throw key_does_not_exist(key);
    } else {
        throw generic_kv_api_failure("rados_backend::get : read failed (key = " + key.to_string() + ')', ret);
    }

    slice.set_size(ret);
    return slice;
}

ssize_t nmfs::kv_backends::rados_backend::get(const nmfs::slice& key, nmfs::slice& value) { // fully read
    uint64_t object_size;
    time_t object_mtime;
    librados::bufferlist buffer_list = librados::bufferlist::static_from_mem(value.data(), value.capacity());
    int ret;

    ret = io_ctx.stat(key.to_string(), &object_size, &object_mtime); // 0 on success, negative error code on failure
    if (ret >= 0) {
        log::information(log_locations::kv_backend_operation)
            << "rados_backend::get : stat(key = " << key.to_string_view() << ") = " << ret << "\n";
    } else if (ret == -ENOENT) {
        throw key_does_not_exist(key);
    } else {
        throw generic_kv_api_failure("rados_backend::get : stat failed (key = " + key.to_string() + ')', ret);
    }

    if (object_size > value.capacity()) {
        throw std::out_of_range("rados_backend::get : capacity of value slice is not enough");
    }

    ret = io_ctx.read(key.to_string(), buffer_list, object_size, 0); // number of bytes read on success, negative error code on failure
    if (ret >= 0) {
        log::information(log_locations::kv_backend_operation)
            << "rados_backend::get : full read(key = " << key.to_string_view() << ", size = " << object_size << ") = " << ret << "\n";
    } else if (ret == -ENOENT) {
        throw key_does_not_exist(key);
    } else {
        throw generic_kv_api_failure("rados_backend::get : full read failed (key = " + key.to_string() + ", size = " + std::to_string(object_size) + ')', ret);
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

    ret = io_ctx.read(key.to_string(), buffer_list, length, offset); // number of bytes read on success, negative error code on failure
    if (ret >= 0) {
        log::information(log_locations::kv_backend_operation)
            << "rados_backend::get : partial read(key = " << key.to_string_view() << ", size = " << length << ", offset = " << offset << ") = " << ret << "\n";
    } else if (ret == -ENOENT) {
        throw key_does_not_exist(key);
    } else {
        throw generic_kv_api_failure("rados_backend::get : partial read failed (key = " + key.to_string() + ", size = " + std::to_string(length) + ", offset = " + std::to_string(offset) + ')', ret);
    }

    value.set_size(ret);
    return ret;
}

ssize_t nmfs::kv_backends::rados_backend::put(const nmfs::slice& key, const nmfs::slice& value) { // fully write
    librados::bufferlist write_buffer = librados::bufferlist::static_from_mem(const_cast<char*>(value.data()), value.size());
    int ret;

    ret = io_ctx.write_full(key.to_string(), write_buffer); // 0 on success, negative error code on failure
    if (ret >= 0) {
        log::information(log_locations::kv_backend_operation)
            << "rados_backend::put : write_full(key = " << key.to_string_view() << ", size = " << value.size() << ") = " << ret << "\n";
    } else {
        throw generic_kv_api_failure("rados_backend::put : write_full failed (key = " + key.to_string() + ", size = " + std::to_string(value.size()) + ')', ret);
    }

    return ret;
}

ssize_t nmfs::kv_backends::rados_backend::put(const nmfs::slice& key, off_t offset, const nmfs::slice& value) { // partial write
    librados::bufferlist buffer_list = librados::bufferlist::static_from_mem(const_cast<char*>(value.data()), value.size());
    int ret;

    ret = io_ctx.write(key.to_string(), buffer_list, value.size(), offset);
    if (ret >= 0) {
        log::information(log_locations::kv_backend_operation)
            << "rados_backend::put : partial write(key = " << key.to_string_view() << ", size = " << value.size() << ", offset = " << offset << ") = " << ret << "\n";
    } else {
        throw generic_kv_api_failure("rados_backend::put : partial write failed (key = " + key.to_string() + ", size = " + std::to_string(value.size()) + ", offset = " + std::to_string(offset) + ')', ret);
    }

    return ret;
}

bool nmfs::kv_backends::rados_backend::exist(const nmfs::slice& key) {
    uint64_t object_size;
    time_t object_mtime;
    int ret;

    ret = io_ctx.stat(key.to_string(), &object_size, &object_mtime); // 0 on success, negative error code on failure
    if (ret >= 0) {
        return true;
    } else if (ret == -ENOENT) {
        return false;
    } else {
        throw generic_kv_api_failure("rados_backend::exist : stat failed (key = " + key.to_string() + ')', ret);
    }
}

void nmfs::kv_backends::rados_backend::remove(const nmfs::slice& key) {
    int ret;

    ret = io_ctx.remove(key.to_string());
    if (ret >= 0) {
        log::information(log_locations::kv_backend_operation)
            << "rados_backend::remove : remove(key = " << key.to_string_view() << ") = " << ret << "\n";
    } else {
        throw generic_kv_api_failure("rados_backend::remove : remove failed (key = " + key.to_string() + ')', ret);
    }
}
