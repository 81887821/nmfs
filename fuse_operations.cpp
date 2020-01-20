#include <sys/stat.h>
#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <cerrno>

#include "fuse_operations.hpp"
#include "memory_slices/slice.hpp"
#include "fuse.hpp"
#include "utils.hpp"
#include "mapper.hpp"
#include "kv_backends/rados_backend.hpp"
#include "structures/metadata.hpp"
#include "structures/super_object.hpp"
#include "exceptions/file_does_not_exist.hpp"

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

    // open root metadata
    std::string root_path("/");
    try {
        auto& root_directory = super_object->cache.open_directory(root_path);
    } catch (nmfs::exceptions::file_does_not_exist&) {
        fuse_context* fuse_context = fuse_get_context();
        auto& root_directory = super_object->cache.create_directory(root_path, fuse_context->uid, fuse_context->gid, 0755 | S_IFDIR);
    }

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

int nmfs::fuse_operations::statfs(const char* path, struct statvfs* stat) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : statfs" << '\n';
#endif
    return 0;
}

int nmfs::fuse_operations::flush(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : flush" << '\n';
#endif
    return 0;
}

int nmfs::fuse_operations::fsync(const char* path, int data_sync, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : fsync" << '\n';
#endif

    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object*>(fuse_context->private_data);
    auto& metadata = super_object.cache.open(path);

    // If the datasync parameter is non-zero, then only the user data should be flushed, not the meta data.

    // TODO : data sync

    if (data_sync == 0) {
        metadata.flush();
    }

    return 0;
}

int nmfs::fuse_operations::fsyncdir(const char* path, int data_sync, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : fsyncdir" << '\n';
#endif
    return 0;
}

int nmfs::fuse_operations::create(const char* path, mode_t mode, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : create" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object*>(fuse_context->private_data);

    try {
        structures::metadata& metadata = super_object.cache.create(path, fuse_context->uid, fuse_context->gid, mode | S_IFREG);
        file_info->fh = reinterpret_cast<uint64_t>(&metadata);

        // add to directory
        std::string current_directory = get_parent_directory(path);
        std::string file_name = get_filename(path);

        auto& directory = super_object.cache.open_directory(current_directory);
        std::cout << "add " << file_name << " to " << current_directory << '\n';
        directory.add_file(file_name);

        directory.flush();
        return 0;
    } catch (nmfs::exceptions::file_already_exist&) {
        return -EEXIST;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT; // Parent does not exist
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::getattr(const char* path, struct stat* stat, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : getattr" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);
    try {
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
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::open(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : open" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object*>(fuse_context->private_data);

    try {
        structures::metadata& metadata = super_object.cache.open(path);
        file_info->fh = reinterpret_cast<uint64_t>(&metadata);

        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::mkdir(const char* path, mode_t mode) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : mkdir" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& new_directory = super_object.cache.create_directory(path, fuse_context->uid, fuse_context->gid, mode | S_IFDIR);

        // add to parent directory
        std::string parent_directory = get_parent_directory(path);
        std::string new_directory_name = get_filename(path);

        auto& directory = super_object.cache.open_directory(parent_directory);
        directory.add_file(new_directory_name);

        directory.flush();

        return 0;
    } catch (nmfs::exceptions::file_already_exist&) {
        return -EEXIST;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT; // Parent does not exist
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::rmdir(const char* path) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : rmdir" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object*>(fuse_context->private_data);

    try {
        structures::metadata& directory_metadata = super_object.cache.open(path);

        if (!S_ISDIR(directory_metadata.mode)) {
            return -ENOTDIR;
        } else if (directory_metadata.size > 0) {
            return -ENOTEMPTY;
        } else {
            //directory.delete();
        }

        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}


int nmfs::fuse_operations::write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : write" << '\n';
#endif
    ssize_t written_size;
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& metadata = file_info? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);
        if (!S_ISREG(metadata.mode)) {
            return -EBADF;
        }

        written_size = metadata.write(buffer, size, offset);

        if (!file_info) {
            super_object->cache.close(path, metadata);
        }

        // should return exactly the number of bytes requested except on error.
        return written_size;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::write_buf(const char* path, struct fuse_bufvec* buffer, off_t offset, struct fuse_file_info* file_info);

int nmfs::fuse_operations::fallocate(const char* path, int mode, off_t offset, off_t length, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : fallocate" << '\n';
#endif
    return 0;
}

int nmfs::fuse_operations::unlink(const char* path) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : unlink" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& metadata = super_object->cache.open(path);
        // remove data blocks
        // metadata.delete();

    } catch (std::runtime_error& e) {
        // TODO
    }

    return 0;
}

int nmfs::fuse_operations::rename(const char* old_path, const char* new_path, unsigned int flags) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : rename" << '\n';
#endif

    return 0;
}

int nmfs::fuse_operations::chmod(const char* path, mode_t mode, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : chmod" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& metadata = file_info? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);

        mode_t file_type = mode | S_IFMT;
        metadata.mode = mode | file_type;

        if (!file_info) {
            super_object->cache.close(path, metadata);
        }

        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::chown(const char* path, uid_t uid, gid_t gid, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : chown" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& metadata = file_info? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);

        metadata.owner = uid;
        metadata.group = gid;

        if (!file_info) {
            super_object->cache.close(path, metadata);
        }

        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::truncate(const char* path, off_t length, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : truncate" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);
    try {
        auto& metadata = file_info? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);
        metadata.truncate(length);
        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : read" << '\n';
#endif

    ssize_t read_size;
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& metadata = file_info? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);
        read_size = metadata.read(buffer, size, offset);

        if (!file_info) {
            super_object->cache.close(path, metadata);
        }

        //should return exactly the number of bytes requested except on EOF or error, otherwise the rest of the data will be substituted with zeroes.
        return read_size;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::read_buf(const char* path, struct fuse_bufvec** buffer, size_t size, off_t offset, struct fuse_file_info* file_info);

int nmfs::fuse_operations::opendir(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : opendir" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        //auto& metadata = (file_info)? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);
        auto& metadata = super_object->cache.open(path); // root directory didn't update file_info->fh

        if (S_ISDIR(metadata.mode)) {
            file_info->fh = reinterpret_cast<uint64_t>(&metadata);
        } else {
            return -ENOTDIR;
        }

        if (!file_info) {
            super_object->cache.close(path, metadata);
        }

        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* file_info, enum fuse_readdir_flags readdir_flags) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : readdir" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& directory = super_object->cache.open_directory(path);

        filler(buffer, ".", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
        filler(buffer, "..", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));

        directory.fill_buffer(fuse_directory_filler(buffer, filler, readdir_flags));

        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::access(const char* path, int mask) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : access" << '\n';
#endif
    return 0;
}

int nmfs::fuse_operations::release(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : release" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);
    auto& metadata = file_info? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);

    super_object->cache.close(path, metadata);
    return 0;
}

int nmfs::fuse_operations::releasedir(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : releasedir" << '\n';
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);
    auto& directory_metadata = file_info? *reinterpret_cast<structures::metadata*>(file_info->fh) : super_object->cache.open(path);

    super_object->cache.close(path, directory_metadata);
    return 0;
}

int nmfs::fuse_operations::utimens(const char*, const struct timespec tv[2], struct fuse_file_info* fi) {
#ifdef DEBUG
    std::cout << '\n' << "__function__call : utimens" << '\n';
#endif

    return 0;
}

::fuse_operations nmfs::fuse_operations::get_fuse_ops() {
    ::fuse_operations operations;
    memset(&operations, 0, sizeof(::fuse_operations));

    operations.init = init;
    operations.destroy = destroy;
    //operations.statfs = statfs;
    //operations.flush = flush;
    //operations.fsync = fsync;
    //operations.fsyncdir = fsyncdir;

    operations.mkdir = mkdir;
    operations.rmdir = rmdir;
    operations.write = write;
    //operations.fallocate = fallocate;
    operations.create = create;
    operations.unlink = unlink;

    //operations.rename = rename;
    operations.chmod = chmod;
    operations.chown = chown;
    operations.truncate = truncate;

    operations.getattr = getattr;
    operations.open = open;
    operations.read = read;
    operations.opendir = opendir;
    operations.readdir = readdir;
    operations.access = access;

    operations.release = release;
    operations.releasedir = releasedir;
    operations.utimens = utimens;
    return operations;
}
