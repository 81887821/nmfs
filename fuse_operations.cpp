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
#include "logger/log.hpp"

using namespace nmfs;

static const std::string_view root_path = std::string_view("/");

void* nmfs::fuse_operations::init(struct fuse_conn_info* info, struct fuse_config* config) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "()\n";
#endif
    auto connect_information = kv_backends::rados_backend::connect_information {};
    auto backend = std::make_unique<kv_backends::rados_backend>(connect_information);
    auto super_object = new structures::super_object(std::move(backend));

    // initialize memory cache and mapper
    nmfs::next_file_handler = 1;

    // open root metadata
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
    log::information(log_locations::fuse_operation) << __func__ << "()\n";
#endif
    auto super_object = reinterpret_cast<structures::super_object*>(private_data);
    delete super_object;
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << "Terminate nmFS successfully.\n";
#endif
}

int nmfs::fuse_operations::statfs(const char* path, struct statvfs* stat) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    return 0;
}

int nmfs::fuse_operations::flush(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    return 0;
}

int nmfs::fuse_operations::fsync(const char* path, int data_sync, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", data_sync = " << data_sync << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", data_sync = " << data_sync << ")\n";
#endif
    return 0;
}

int nmfs::fuse_operations::create(const char* path, mode_t mode, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", mode = 0" << std::oct << mode << ")\n";
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
        log::information(log_locations::directory_operation) << "add " << file_name << " to " << current_directory << '\n';
        directory.add_file(file_name, metadata);

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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", mode = 0" << std::oct << mode << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& new_directory = super_object.cache.create_directory(path, fuse_context->uid, fuse_context->gid, mode | S_IFDIR);

        // add to parent directory
        std::string parent_directory = get_parent_directory(path);
        std::string new_directory_name = get_filename(path);

        auto& directory = super_object.cache.open_directory(parent_directory);
        directory.add_file(new_directory_name, new_directory.directory_metadata);

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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", size = 0x" << std::hex << size << ", offset = 0x" << offset << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", offset = 0x" << std::hex << offset << ", length = 0x" << length << ")\n";
#endif
    return 0;
}

int nmfs::fuse_operations::unlink(const char* path) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(old_path = " << old_path << ", new_path = " << new_path << ", flags = " << flags << ")\n";
#endif

    return 0;
}

int nmfs::fuse_operations::chmod(const char* path, mode_t mode, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", mode = 0" << std::oct << mode << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path <<  ", uid = " << uid << ", gid = " << gid << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", length = 0x" << std::hex << length << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", size = 0x" << std::hex << size << ", offset = 0x" << offset << ")\n";
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
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& directory = super_object->cache.open_directory(path);

        file_info->fh = reinterpret_cast<uint64_t>(&directory);

        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (nmfs::exceptions::is_not_directory&) {
        return -ENOTDIR;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* file_info, enum fuse_readdir_flags readdir_flags) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    try {
        auto& directory = file_info? *reinterpret_cast<structures::directory<super_object::indexing_type>*>(file_info->fh) : super_object->cache.open_directory(path);

        filler(buffer, ".", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
        filler(buffer, "..", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));

        directory.fill_buffer(fuse_directory_filler(buffer, filler, readdir_flags));

        if (!file_info) {
            super_object->cache.close_directory(path, directory);
        }

        return 0;
    } catch (nmfs::exceptions::file_does_not_exist&) {
        return -ENOENT;
    } catch (nmfs::exceptions::is_not_directory&) {
        return -ENOTDIR;
    } catch (std::exception&) {
        return -EIO;
    }
}

int nmfs::fuse_operations::access(const char* path, int mask) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", mask = 0" << std::oct << mask << ")\n";
#endif
    return 0;
}

int nmfs::fuse_operations::release(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    if (file_info) {
        auto& metadata = *reinterpret_cast<structures::metadata*>(file_info->fh);
        super_object->cache.close(path, metadata);
    }

    return 0;
}

int nmfs::fuse_operations::releasedir(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto super_object = reinterpret_cast<structures::super_object*>(fuse_context->private_data);

    if (file_info) {
        auto& directory = *reinterpret_cast<structures::directory<super_object::indexing_type>*>(file_info->fh);
        super_object->cache.close_directory(path, directory);
    }

    return 0;
}

int nmfs::fuse_operations::utimens(const char* path, const struct timespec tv[2], struct fuse_file_info* fi) {
#ifdef DEBUG
    // TODO: print struct timespec
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", tv[0], tv[1]" << ")\n";
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
