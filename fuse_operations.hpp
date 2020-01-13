#ifndef NMFS_FUSE_OPERATIONS_HPP
#define NMFS_FUSE_OPERATIONS_HPP

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <rados/librados.hpp>

namespace nmfs::fuse_operations {

void* init(struct fuse_conn_info* info, struct fuse_config *config);
void destroy(void* private_data);
int statfs(const char* path, struct statvfs* stat);
int flush(const char* path, struct fuse_file_info* file_info);
int fsync(const char* path, int data_sync, struct fuse_file_info* file_info);
int fsyncdir(const char* path, int data_sync, struct fuse_file_info* file_info);

int mkdir(const char* path, mode_t mode);
int rmdir(const char* path);
int write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info);
int write_buf(const char* path, struct fuse_bufvec* buffer, off_t offset, struct fuse_file_info* file_info);
int fallocate(const char* path, int mode, off_t offset, off_t length, struct fuse_file_info* file_info);
int create(const char* path, mode_t mode, struct fuse_file_info* file_info);
int unlink(const char* path);

int rename(const char* old_path, const char* new_path, unsigned int flags);
int chmod(const char* path, mode_t mode, struct fuse_file_info* file_info);
int chown(const char* path, uid_t uid, gid_t gid, struct fuse_file_info* file_info);
int truncate(const char* path, off_t length, struct fuse_file_info* file_info);

int getattr(const char* path, struct stat* stat, struct fuse_file_info* file_info);
int open(const char* path, struct fuse_file_info* file_info);
int read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info);
int opendir(const char* path, struct fuse_file_info* file_info);
int readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* file_info, enum fuse_readdir_flags readdir_flags);
int access(const char* path, int mask);
int read_buf(const char* path, struct fuse_bufvec** buffer, size_t size, off_t offset, struct fuse_file_info* file_info);

int release(const char* path, struct fuse_file_info* file_info);
int releasedir(const char* path, struct fuse_file_info* file_info);

::fuse_operations get_fuse_ops();

} // namespace nmfs::fuse_operations

#endif 
