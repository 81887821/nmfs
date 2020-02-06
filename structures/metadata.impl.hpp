#ifndef NMFS_STRUCTURES_METADATA_IMPL_HPP
#define NMFS_STRUCTURES_METADATA_IMPL_HPP

#include <utility>
#include "metadata.hpp"
#include "../logger/log.hpp"
#include "../logger/write_bytes.hpp"
#include "../memory_slices/borrower_slice.hpp"
#include "../kv_backends/exceptions/key_does_not_exist.hpp"
#include "../kv_backends/exceptions/generic_kv_api_failure.hpp"
#include "utils/data_object_key.hpp"
#include "super_object.impl.hpp"

namespace nmfs::structures {

template<typename indexing>
metadata<indexing>::metadata(super_object<indexing>& super, owner_slice key, uid_t owner, gid_t group, mode_t mode)
    : context(super),
      key(std::move(key)),
      open_count(1),
      link_count(1),
      owner(owner),
      group(group),
      mode(mode),
      size(0),
      last_close(std::chrono::system_clock::now()),
      dirty(true),
      mutex(std::make_shared<std::shared_mutex>()) {
    if (timespec_get(&atime, TIME_UTC) != TIME_UTC) {
        throw std::runtime_error("timespec_get failed.");
    }
    mtime = atime;
    ctime = atime;
}

template<typename indexing>
metadata<indexing>::metadata(super_object<indexing>& super, owner_slice key, const on_disk::metadata* on_disk_structure)
    : context(super),
      key(std::move(key)),
      open_count(1),
      link_count(on_disk_structure->link_count),
      owner(on_disk_structure->owner),
      group(on_disk_structure->group),
      mode(on_disk_structure->mode),
      size(on_disk_structure->size),
      atime(on_disk_structure->atime),
      mtime(on_disk_structure->mtime),
      ctime(on_disk_structure->ctime),
      last_close(std::chrono::system_clock::now()),
      mutex(std::make_shared<std::shared_mutex>()) {
}

template<typename indexing>
metadata<indexing>::metadata(metadata&& other, nmfs::owner_slice key)
    : context(other.context),
      key(0), /* Key is needed for moving data, so key will moved later */
      open_count(0),
      link_count(other.link_count),
      owner(other.owner),
      group(other.group),
      mode(other.mode),
      size(other.size),
      atime(other.atime),
      mtime(other.mtime),
      ctime(other.ctime),
      valid(other.valid),
      last_close(std::chrono::system_clock::now()),
      dirty(true),
      mutex(std::make_shared<std::shared_mutex>()) {
    other.dirty = false;
    other.move_data(key);
    if (key != other.key) {
        other.remove();
    }
    other.valid = false;
    this->key = std::move(key);
}

template<typename indexing>
metadata<indexing>::metadata(metadata&& other, nmfs::owner_slice key, const slice& new_data_key_base)
    : context(other.context),
      key(0), /* Key is needed for moving data, so key will moved later */
      open_count(0),
      link_count(other.link_count),
      owner(other.owner),
      group(other.group),
      mode(other.mode),
      size(other.size),
      atime(other.atime),
      mtime(other.mtime),
      ctime(other.ctime),
      valid(other.valid),
      last_close(std::chrono::system_clock::now()),
      dirty(true),
      mutex(std::make_shared<std::shared_mutex>()) {
    other.dirty = false;
    other.move_data(new_data_key_base);
    if (key != other.key) {
        other.remove();
    }
    other.valid = false;
    this->key = std::move(key);
}

template<typename indexing>
metadata<indexing>::metadata(metadata&& other) noexcept
    : context(other.context),
      key(std::move(other.key)),
      open_count(other.open_count),
      link_count(other.link_count),
      owner(other.owner),
      group(other.group),
      mode(other.mode),
      size(other.size),
      atime(other.atime),
      mtime(other.mtime),
      ctime(other.ctime),
      valid(other.valid),
      last_close(other.last_close),
      dirty(other.dirty),
      mutex(std::move(other.mutex)) {
    other.valid = false;
}

template<typename indexing>
ssize_t metadata<indexing>::write(const byte* buffer, size_t size_to_write, off_t offset) {
    log::information(log_locations::file_data_operation) << std::showbase << std::hex << "(" << this << ") " << __func__ << "(size = " << size_to_write << ", offset = " << offset << ")\n";
    log::information(log_locations::file_data_content) << std::showbase << std::hex << "(" << this << ") " << __func__ << " = " << write_bytes(buffer, size_to_write) << '\n';

    auto data_key = nmfs::structures::utils::data_object_key(key, static_cast<uint32_t>(offset / context.maximum_object_size));
    auto offset_in_object = static_cast<uint32_t>(offset % context.maximum_object_size);
    uint32_t remain_size_in_object = context.maximum_object_size - offset_in_object;
    size_t remain_size_to_write = size_to_write;

    if (offset + size_to_write > size) {
        size = offset + size_to_write;
        dirty = true;
    }

    while (remain_size_to_write > 0) {
        size_t size_to_write_in_object = std::min<size_t>(remain_size_in_object, remain_size_to_write);
        DECLARE_CONST_BORROWER_SLICE(value, buffer, size_to_write_in_object);

        context.backend->put(data_key, offset_in_object, value);
        offset_in_object = 0;
        remain_size_in_object = context.maximum_object_size;
        remain_size_to_write -= size_to_write_in_object;
        buffer += size_to_write_in_object;
        data_key.increase_index();
    }

    return size_to_write;
}

template<typename indexing>
ssize_t metadata<indexing>::read(byte* buffer, size_t size_to_read, off_t offset) const {
    log::information(log_locations::file_data_operation) << std::showbase << std::hex << "(" << this << ") " << __func__ << "(size = " << size_to_read << ", offset = " << offset << ")\n";

    auto data_key = nmfs::structures::utils::data_object_key(key, static_cast<uint32_t>(offset / context.maximum_object_size));
    auto offset_in_object = static_cast<uint32_t>(offset % context.maximum_object_size);
    uint32_t remain_size_in_object = context.maximum_object_size - offset_in_object;

    if (size_to_read + offset > size) {
        size_to_read = size - offset;
    }

    size_t remain_size_to_read = size_to_read;

    while (remain_size_to_read > 0) {
        size_t size_to_read_in_object = std::min<size_t>(remain_size_in_object, remain_size_to_read);
        auto value = borrower_slice(buffer, size_to_read_in_object);
        ssize_t read_size;

        try {
            read_size = context.backend->get(data_key, offset_in_object, size_to_read_in_object, value);
        } catch (kv_backends::exceptions::key_does_not_exist&) {
            read_size = 0;
        }

        if (read_size < size_to_read_in_object) {
            std::fill(buffer + read_size, buffer + size_to_read_in_object, 0);
        }
        offset_in_object = 0;
        remain_size_in_object = context.maximum_object_size;
        remain_size_to_read -= size_to_read_in_object;
        buffer += size_to_read_in_object;
        data_key.increase_index();
    }

    log::information(log_locations::file_data_content) << std::showbase << std::hex << "(" << this << ") " << __func__ << " = " << write_bytes(buffer - size_to_read, size_to_read) << '\n';

    return size_to_read;
}

template<typename indexing>
void metadata<indexing>::truncate(off_t new_size) {
    log::information(log_locations::file_data_operation) << std::showbase << std::hex << "(" << this << ") " << __func__ << "(new_size = " << new_size << ")\n";

    if (new_size != size) {
        if (new_size < size) {
            auto first_index = static_cast<uint32_t>(new_size / context.maximum_object_size + new_size % context.maximum_object_size? 1 : 0);
            auto last_index = static_cast<uint32_t>(size / context.maximum_object_size);

            remove_data_objects(first_index, last_index);
        }
        size = new_size;
        dirty = true;
    }
}

template<typename indexing>
void metadata<indexing>::remove_data_objects(uint32_t index_from, uint32_t index_to) {
    log::information(log_locations::file_data_operation) << std::showbase << std::hex << __func__ << "(index_from = " << index_from << ", index_to = " << index_to << ")\n";
    auto data_key = nmfs::structures::utils::data_object_key(key, index_from);
    for (uint32_t i = index_from; i <= index_to; i++) {
        data_key.update_index(i);
        context.backend->remove(data_key);
    }
}

template<typename indexing>
void metadata<indexing>::reload() {
    // TODO
}

template<typename indexing>
void metadata<indexing>::remove() {
    log::information(log_locations::file_data_operation) << std::showbase << std::hex << "(" << this << ") " << __func__ << "()\n";

    if (valid) {
        if (size > 0) {
            remove_data_objects(0, size / context.maximum_object_size);
        }
        context.backend->remove(key);
    }
    dirty = false;
    valid = false;
}

template<typename indexing>
void metadata<indexing>::to_on_disk_metadata(on_disk::metadata& on_disk_metadata) const {
    on_disk_metadata.link_count = link_count;
    on_disk_metadata.owner = owner;
    on_disk_metadata.group = group;
    on_disk_metadata.mode = mode;
    on_disk_metadata.size = size;
    on_disk_metadata.atime = atime;
    on_disk_metadata.mtime = mtime;
    on_disk_metadata.ctime = ctime;
}

template<typename indexing>
constexpr struct stat metadata<indexing>::to_stat() const {
    struct stat stat{
        .st_nlink = link_count,
        .st_mode = mode,
        .st_uid = owner,
        .st_gid = group,
        .st_size = static_cast<off_t>(size),
        .st_atim = atime,
        .st_mtim = mtime,
        .st_ctim = ctime,
    };
    
    return stat;
}

}

#endif //NMFS_STRUCTURES_METADATA_IMPL_HPP
