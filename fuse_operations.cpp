#include <sys/stat.h>
#include <linux/fs.h>
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
#include "exceptions/file_does_not_exist.hpp"
#include "logger/log.hpp"
#include "local_caches/cache_store.impl.hpp"
#include "local_caches/caching_policy/all.impl.hpp"
#include "structures/indexing_types/all.impl.hpp"
#include "structures/directory.impl.hpp"
#include "structures/metadata.impl.hpp"
#include "structures/super_object.impl.hpp"
#include "local_caches/utils/no_lock.hpp"

using namespace nmfs;
using indexing = configuration::indexing;

static const std::string_view root_path = std::string_view("/");

void* nmfs::fuse_operations::init(struct fuse_conn_info* info, struct fuse_config* config) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "()\n";
#endif
    auto connect_information = kv_backends::rados_backend::connect_information {};
    auto backend = std::make_unique<kv_backends::rados_backend>(connect_information);
    auto super_object = new structures::super_object<indexing>(std::move(backend));

    // initialize memory cache and mapper
    nmfs::next_file_handler = 1;

    // open root metadata
    try {
        auto& root_directory = super_object->cache->open_directory<no_lock>(root_path).unlock_and_release_directory();
    } catch (nmfs::exceptions::file_does_not_exist&) {
        fuse_context* fuse_context = fuse_get_context();
        auto& root_directory = super_object->cache->create_directory<no_lock>(root_path, fuse_context->uid, fuse_context->gid, 0755 | S_IFDIR).unlock_and_release_directory();
    }

    // set fuse_context->private_data to super_object instance
    return super_object;
}

void nmfs::fuse_operations::destroy(void* private_data) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "()\n";
#endif
    auto super_object = reinterpret_cast<structures::super_object<indexing>*>(private_data);
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
    auto& super_object = *static_cast<structures::super_object<indexing>*>(fuse_context->private_data);
    auto open_context = super_object.cache->open<std::unique_lock>(path);
    auto& metadata = open_context.metadata;

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
    auto& super_object = *static_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto open_context = super_object.cache->create<std::unique_lock>(path, fuse_context->uid, fuse_context->gid, mode | S_IFREG);

        // add to directory
        std::string_view parent_path = get_parent_directory(path);
        std::string_view file_name = get_filename(path);

        auto parent_open_context = super_object.cache->open_directory<std::unique_lock>(parent_path);
        auto& parent_directory = parent_open_context.directory;
        parent_directory.add_file(file_name, open_context.metadata);

        parent_directory.flush();
        // Create performs "create and open a file", so we don't close metadata here
        file_info->fh = reinterpret_cast<uint64_t>(&open_context.unlock_and_release());
        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::getattr(const char* path, struct stat* stat, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);
    try {
        mode_t type = file_info? S_IFREG : indexing::get_type(super_object, path);

        if (S_ISREG(type)) {
            auto open_context = file_info? nmfs::open_context<indexing, std::shared_lock>(path, *reinterpret_cast<structures::metadata<indexing>*>(file_info->fh)) : super_object.cache->open<std::shared_lock>(path);
            auto& metadata = open_context.metadata;

            *stat = metadata.to_stat();

            if (file_info) {
                open_context.unlock_and_release();
            }
        } else if (S_ISDIR(type)) {
            // if directory
            auto open_context = super_object.cache->open_directory<std::shared_lock>(path);
            auto& directory = open_context.directory;
            auto& metadata = directory.directory_metadata;

            *stat = metadata.to_stat();
        } else {
            log::error(log_locations::fuse_operation) << std::showbase << __func__ << " failed: Unsupported file type " << std::hex << type << '\n';
            return -EFAULT;
        }

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::open(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        structures::metadata<indexing>& metadata = super_object.cache->open<no_lock>(path).unlock_and_release();
        file_info->fh = reinterpret_cast<uint64_t>(&metadata);
        log::information(log_locations::fuse_operation) << std::hex << std::showbase << __func__ << ": " << path << " = " << &metadata << '\n';

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::mkdir(const char* path, mode_t mode) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", mode = 0" << std::oct << mode << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto new_directory_open_context = super_object.cache->create_directory<std::unique_lock>(path, fuse_context->uid, fuse_context->gid, mode | S_IFDIR);
        auto& new_directory = new_directory_open_context.directory;

        // add to parent directory
        std::string_view parent_path = get_parent_directory(path);
        std::string_view new_directory_name = get_filename(path);

        auto parent_open_context = super_object.cache->open_directory<std::unique_lock>(parent_path);
        auto& parent_directory = parent_open_context.directory;
        parent_directory.add_file(new_directory_name, new_directory.directory_metadata);

        parent_directory.flush();
        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::rmdir(const char* path) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *static_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto open_context = super_object.cache->open_directory<std::unique_lock>(path);
        auto& directory = open_context.directory;

        if (directory.number_of_files() > 0) {
            return -ENOTEMPTY;
        } else {
            std::string_view parent_path = get_parent_directory(path);
            auto parent_open_context = super_object.cache->open_directory<std::unique_lock>(parent_path);
            auto& parent_directory = parent_open_context.directory;

            parent_directory.remove_file(get_filename(path));

            super_object.cache->remove_directory(std::move(open_context));
            return 0;
        }
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", size = 0x" << std::hex << size << ", offset = 0x" << offset << ")\n";
#endif
    ssize_t written_size;
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto open_context = file_info? nmfs::open_context<indexing, std::unique_lock>(path, *reinterpret_cast<structures::metadata<indexing>*>(file_info->fh)) : super_object.cache->open<std::unique_lock>(path);
        auto& metadata = open_context.metadata;
        if (!S_ISREG(metadata.mode)) {
            return -EBADF;
        }

        written_size = metadata.write(buffer, size, offset);

        if (file_info) {
            open_context.unlock_and_release();
        }

        // should return exactly the number of bytes requested except on error.
        return written_size;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
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
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto open_context = super_object.cache->open<std::unique_lock>(path);
        auto& metadata = open_context.metadata;

        std::string_view parent_path = get_parent_directory(path);
        auto parent_open_context = super_object.cache->open_directory<std::unique_lock>(parent_path);
        auto& parent_directory = parent_open_context.directory;

        parent_directory.remove_file(get_filename(path));
        super_object.cache->remove(std::move(open_context));

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::rename(const char* old_path, const char* new_path, unsigned int flags) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(old_path = " << old_path << ", new_path = " << new_path << ", flags = " << flags << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    if (flags & RENAME_EXCHANGE) {
        return -ENOTSUP;
    }

    try {
        mode_t type = indexing::get_type(super_object, old_path);
        bool target_exist = true;
        mode_t target_type;

        try {
            target_type = indexing::get_type(super_object, new_path);
        } catch (nmfs::exceptions::file_does_not_exist&) {
            target_exist = false;
        }

        if ((flags & RENAME_NOREPLACE) && target_exist) {
            return -EEXIST;
        } else if (S_ISDIR(type)) {
            if (target_exist) {
                if (S_ISDIR(target_type)) {
                    auto target_open_context = super_object.cache->open_directory<std::unique_lock>(new_path);
                    auto& target_directory = target_open_context.directory;

                    if (target_directory.empty()) {
                        super_object.cache->remove_directory(std::move(target_open_context));
                        /* Proceeds to moving directory */
                    } else {
                        return -ENOTEMPTY;
                    }
                } else {
                    return -ENOTDIR;
                }
            }
            /* Moving directory */
            super_object.cache->move_directory(old_path, new_path);
            /* Proceeds to directory management */
        } else if (S_ISREG(type)) {
            if (target_exist) {
                if (!S_ISDIR(target_type)) {
                    auto target_open_context = super_object.cache->open<std::unique_lock>(new_path);
                    super_object.cache->remove(std::move(target_open_context));
                    /* Proceeds to moving regular file */
                } else {
                    return -EISDIR;
                }
            }
            /* Moving regular file */
            super_object.cache->move(old_path, new_path);
            /* Proceeds to directory management */
        } else {
            throw nmfs::exceptions::type_not_supported(type);
        }

        /* Directory management */
        std::string_view old_parent_path = get_parent_directory(old_path);
        std::string_view new_parent_path = get_parent_directory(new_path);
        auto old_parent_open_context = super_object.cache->open_directory<std::unique_lock>(old_parent_path);
        auto& old_parent_directory = old_parent_open_context.directory;

        if (old_parent_path == new_parent_path) {
            old_parent_directory.move_entry(old_path, new_path, old_parent_directory);
        } else {
            auto new_parent_open_context = super_object.cache->open_directory<std::unique_lock>(new_parent_path);
            auto& new_parent_directory = new_parent_open_context.directory;

            old_parent_directory.move_entry(old_path, new_path, new_parent_directory);
        }

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::chmod(const char* path, mode_t mode, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", mode = 0" << std::oct << mode << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto open_context = file_info? nmfs::open_context<indexing, std::unique_lock>(path, *reinterpret_cast<structures::metadata<indexing>*>(file_info->fh)) : super_object.cache->open<std::unique_lock>(path);
        auto& metadata = open_context.metadata;

        mode_t file_type = mode & S_IFMT;
        metadata.mode = mode | file_type;
        metadata.dirty = true;

        if (file_info) {
            open_context.unlock_and_release();
        }

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::chown(const char* path, uid_t uid, gid_t gid, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", uid = " << uid << ", gid = " << gid << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto open_context = file_info? nmfs::open_context<indexing, std::unique_lock>(path, *reinterpret_cast<structures::metadata<indexing>*>(file_info->fh)) : super_object.cache->open<std::unique_lock>(path);
        auto& metadata = open_context.metadata;

        metadata.owner = uid;
        metadata.group = gid;
        metadata.dirty = true;

        if (file_info) {
            open_context.unlock_and_release();
        }

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::truncate(const char* path, off_t length, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", length = 0x" << std::hex << length << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);
    try {
        auto open_context = file_info? nmfs::open_context<indexing, std::unique_lock>(path, *reinterpret_cast<structures::metadata<indexing>*>(file_info->fh)) : super_object.cache->open<std::unique_lock>(path);
        auto& metadata = open_context.metadata;

        metadata.truncate(length);

        if (file_info) {
            open_context.unlock_and_release();
        }

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ", size = 0x" << std::hex << size << ", offset = 0x" << offset << ")\n";
#endif

    ssize_t read_size;
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto open_context = file_info? nmfs::open_context<indexing, std::shared_lock>(path, *reinterpret_cast<structures::metadata<indexing>*>(file_info->fh)) : super_object.cache->open<std::shared_lock>(path);
        auto& metadata = open_context.metadata;
        read_size = metadata.read(buffer, size, offset);

        if (file_info) {
            open_context.unlock_and_release();
        }

        //should return exactly the number of bytes requested except on EOF or error, otherwise the rest of the data will be substituted with zeroes.
        return read_size;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::read_buf(const char* path, struct fuse_bufvec** buffer, size_t size, off_t offset, struct fuse_file_info* file_info);

int nmfs::fuse_operations::opendir(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto& directory = super_object.cache->open_directory<no_lock>(path).unlock_and_release_directory();

        file_info->fh = reinterpret_cast<uint64_t>(&directory);

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return -EIO;
    }
}

int nmfs::fuse_operations::readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* file_info, enum fuse_readdir_flags readdir_flags) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    try {
        auto open_context = file_info? directory_open_context<indexing, std::shared_lock>(path, *reinterpret_cast<structures::directory<indexing>*>(file_info->fh)) : super_object.cache->open_directory<std::shared_lock>(path);
        auto& directory = open_context.directory;

        filler(buffer, ".", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
        filler(buffer, "..", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));

        directory.fill_buffer(fuse_directory_filler(buffer, filler, readdir_flags));

        if (file_info) {
            open_context.unlock_and_release_directory();
        }

        return 0;
    } catch (nmfs::exceptions::nmfs_exception& e) {
        log::debug(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
        return e.error_code();
    } catch (std::exception& e) {
        log::error(log_locations::fuse_operation) << __func__ << " failed: " << e.what() << '\n';
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
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    if (file_info) {
        auto open_context = nmfs::open_context<indexing, no_lock>(path, *reinterpret_cast<structures::metadata<indexing>*>(file_info->fh));
    }

    return 0;
}

int nmfs::fuse_operations::releasedir(const char* path, struct fuse_file_info* file_info) {
#ifdef DEBUG
    log::information(log_locations::fuse_operation) << __func__ << "(path = " << path << ")\n";
#endif
    fuse_context* fuse_context = fuse_get_context();
    auto& super_object = *reinterpret_cast<structures::super_object<indexing>*>(fuse_context->private_data);

    if (file_info) {
        auto open_context = nmfs::directory_open_context<indexing, no_lock>(path, *reinterpret_cast<structures::directory<indexing>*>(file_info->fh));
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

    operations.rename = rename;
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
