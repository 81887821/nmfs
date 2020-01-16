#include <iostream>
#include <string>
#include <cstring>
#include <rados/librados.hpp>
#include <memory>

#include "fuse_operations.hpp"
#include "memory_slices/slice.hpp"
#include "constants.hpp"
#include "fuse.hpp"
#include "utils.hpp"
#include "mapper.hpp"
#include "kv_backends/rados_backend.hpp"
#include "structures/metadata.hpp"
#include "structures/super_object.hpp"

void* nmfs::fuse_operations::init(struct fuse_conn_info* info, struct fuse_config* config) {
    int err;
#ifdef DEBUG
    std::cout << '\n' << "__function__call : init" << '\n';
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

    // initialize fuse context and set kv_backend
    fuse_context* fuse_context = fuse_get_context();
    fuse_context->private_data = new nmfs::structures::super_object();

    // initialize memory cache and mapper
    nmfs::next_file_handler = 1;

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
    std::cout << '\n' << "__function__call : create" << '\n';
#endif

    return 0;
}

int getattr(const char* path, struct stat* stat, struct fuse_file_info* file_info){
#ifdef DEBUG
    std::cout << '\n' << "__function__call : getattr" << '\n';
#endif
    fuse_context *fuse_context = fuse_get_context();
    std::shared_ptr<nmfs::structures::metadata> metadata = get_metadata(fuse_context, path); //////

    std::memset(stat, 0, sizeof(struct stat));
    stat->st_nlink = metadata->link_count;
    stat->st_mode = metadata->mode;
    stat->st_uid = metadata->owner;
    stat->st_gid = metadata->group;
    stat->st_size = metadata->size;
    stat->st_atim = metadata->atime;
    stat->st_mtim = metadata->mtime;
    stat->st_ctim = metadata->ctime;

    return 0;
}

int open(const char* path, struct fuse_file_info* file_info){
#ifdef DEBUG
    std::cout << '\n' << "__function__call : open" << '\n';
#endif
    uint64_t file_handler_num;
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<nmfs::structures::super_object*>(fuse_context->private_data);


    if(file_info->fh > 0)
        return 0;

    // make key slice
    auto key = nmfs::make_key(path, nmfs::key_mode);

    try {
        auto value = super_object.backend->get(*key);

        //nmfs::structures::metadata* metadata;
        auto on_disk_metadata = reinterpret_cast<nmfs::structures::on_disk::metadata*>(value.data());

        nmfs::structures::metadata metadata(super_object, path, *on_disk_metadata);

        file_handler_num = nmfs::next_file_handler;
        nmfs::next_file_handler++;

        nmfs::file_handler_map[std::string(key->data())] = file_handler_num;

        // caching
    } catch (std::runtime_error& e) {

        // create metadata
        std::unique_ptr<nmfs::structures::metadata> metadata = std::make_unique<nmfs::structures::metadata>(super_object, path, fuse_context, S_IFREG | 0755);

        // write metadat on disk
        metadata->sync();

        file_handler_num = nmfs::next_file_handler;
        nmfs::next_file_handler++;

        nmfs::file_handler_map[std::string(key->data())] = file_handler_num;
    }

    file_info->fh = file_handler_num;

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

