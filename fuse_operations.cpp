#include <sys/stat.h>
#include <iostream>
#include <string>
#include <cstring>
#include <memory>

#include "fuse_operations.hpp"
#include "memory_slices/slice.hpp"
#include "fuse.hpp"
#include "utils.hpp"
#include "mapper.hpp"
#include "kv_backends/rados_backend.hpp"
#include "structures/metadata.hpp"
#include "structures/super_object.hpp"

using namespace nmfs;

void* nmfs::fuse_operations::init(struct fuse_conn_info* info, struct fuse_config* config) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : init" << '\n';
#endif
    auto connect_information = kv_backends::rados_backend::connect_information {};
    auto backend = std::make_unique<kv_backends::rados_backend>(connect_information);
    auto super_object = new structures::super_object(std::move(backend));

    // initialize memory cache and mapper
    nmfs::next_file_handler = 1;

    // set fuse_context->private_data to super_object instance
    return super_object;
}

void nmfs::fuse_operations::destroy(void* private_data) {
#ifdef DEBUG
    std::cout << "__function__call : delete" << std::endl;
#endif
    auto super_object = reinterpret_cast<structures::super_object*>(private_data);
    delete super_object;
#ifdef DEBUG
    std::cout << "Terminate nmFS successfully." << std::endl;
#endif
}

int nmfs::fuse_operations::statfs(const char* path, struct statvfs* stat);
int nmfs::fuse_operations::flush(const char* path, struct fuse_file_info* file_info);
int nmfs::fuse_operations::fsync(const char* path, int data_sync, struct fuse_file_info* file_info);
int nmfs::fuse_operations::fsyncdir(const char* path, int data_sync, struct fuse_file_info* file_info);

int nmfs::fuse_operations::create(const char* path, mode_t mode, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : create" << '\n';
#endif

    return 0;
}

int getattr(const char* path, struct stat* stat, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : getattr" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_get_context()->private_data);
    auto& metadata = file_info? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);

    std::memset(stat, 0, sizeof(struct stat));
    stat->st_nlink = metadata.link_count;
    stat->st_mode = metadata.mode;
    stat->st_uid = metadata.owner;
    stat->st_gid = metadata.group;
    stat->st_size = metadata.size;
    stat->st_atim = metadata.atime;
    stat->st_mtim = metadata.mtime;
    stat->st_ctim = metadata.ctime;

    if (!file_info) {
        super_object->cache.close(path, metadata);
    }

    return 0;
}

int open(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : open" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object*>(fuse_context->private_data);

    try {
        structures::metadata& metadata = super_object.cache.open(path);
        file_info->fh = reinterpret_cast<uint64_t>(&metadata);
        return 0;
    } catch (std::runtime_error& e) {
        return nmfs::fuse_operations::create(path, 0644 | S_IFREG, file_info);
    }
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
