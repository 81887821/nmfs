#include <iostream>
#include <string>
#include <rados/librados.hpp>


#include "fuse_operations.hpp"
#include "memory_slices/slice.hpp"

#include "constants.hpp"

void* nmfs::fuse_operations::init(struct fuse_conn_info* info, struct fuse_config *config) {
    int err;
#ifdef DEBUG
    std::cout << "__function__call : init" << std::endl;
#endif


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

#ifdef DEBUG
    std::cout << "__function__call : delete" << std::endl;
#endif
    io_ctx.close();
    cluster.shutdown();

#ifdef DEBUG
    std::cout << "Terminate nmFS successfully." << std::endl;
#endif

}

int nmfs::fuse_operations::statfs(const char* path, struct statvfs* stat);
int nmfs::fuse_operations::flush(const char* path, struct fuse_file_info* file_info);
int nmfs::fuse_operations::fsync(const char* path, int data_sync, struct fuse_file_info* file_info);
int nmfs::fuse_operations::fsyncdir(const char* path, int data_sync, struct fuse_file_info* file_info);

int nmfs::fuse_operations::create(const char* path, mode_t mode, struct fuse_file_info* file_info){
#ifdef DEBUG
    std::cout << "__function__call : create" << std::endl;
#endif

    return 0;
}
int getattr(const char* path, struct stat* stat, struct fuse_file_info* file_info){
#ifdef DEBUG
    std::cout << "__function__call : getattr" << std::endl;
#endif

    return 0;
}

int open(const char* path, struct fuse_file_info* file_info){
#ifdef DEBUG
    std::cout << "__function__call : open" << std::endl;
#endif
    return 0;
}
int nmfs::fuse_operations::mkdir(const char* path, mode_t mode);
int nmfs::fuse_operations::rmdir(const char* path);
int nmfs::fuse_operations::write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info);
int nmfs::fuse_operations::write_buf(const char* path, struct fuse_bufvec* buffer, off_t offset, struct fuse_file_info* file_info);
int nmfs::fuse_operations::fallocate(const char* path, int mode, off_t offset, off_t length, struct fuse_file_info* file_info);
int nmfs::fuse_operations::unlink(const char* path);
int nmfs::fuse_operations::rename(const char* old_path, const char* new_path, unsigned int flags);
int nmfs::fuse_operations::chmod(const char* path, mode_t mode, struct fuse_file_info* file_info);
int nmfs::fuse_operations::chown(const char* path, uid_t uid, gid_t gid, struct fuse_file_info* file_info);
int nmfs::fuse_operations::truncate(const char* path, off_t length, struct fuse_file_info* file_info);
int nmfs::fuse_operations::read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info);
int nmfs::fuse_operations::opendir(const char* path, struct fuse_file_info* file_info);
int nmfs::fuse_operations::readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* file_info, enum fuse_readdir_flags readdir_flags);
int nmfs::fuse_operations::access(const char* path, int mask);
int nmfs::fuse_operations::read_buf(const char* path, struct fuse_bufvec** buffer, size_t size, off_t offset, struct fuse_file_info* file_info);
int nmfs::fuse_operations::release(const char* path, struct fuse_file_info* file_info);
int nmfs::fuse_operations::releasedir(const char* path, struct fuse_file_info* file_info);

