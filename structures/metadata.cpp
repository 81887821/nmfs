#include <utility>
#include "metadata.hpp"
#include "../memory_slices/borrower_slice.hpp"
#include "utils/data_object_key.hpp"

using namespace nmfs::structures;

metadata::metadata(super_object& super, std::string path, fuse_context* fuse_context, mode_t mode)
    : context(super),
      path(std::move(path)),
      link_count(1),
      owner(fuse_context->uid),
      group(fuse_context->gid),
      mode(mode),
      size(0),
      dirty(true) {
    if (timespec_get(&atime, TIME_UTC) != TIME_UTC) {
        throw std::runtime_error("timespec_get failed.");
    }
    mtime = atime;
    ctime = atime;
}

metadata::metadata(super_object& super, std::string path, const on_disk::metadata& on_disk_structure)
    : context(super),
      path(std::move(path)),
      link_count(on_disk_structure.link_count),
      owner(on_disk_structure.owner),
      group(on_disk_structure.group),
      mode(on_disk_structure.mode),
      size(on_disk_structure.size),
      atime(on_disk_structure.atime),
      mtime(on_disk_structure.mtime),
      ctime(on_disk_structure.ctime) {
}

ssize_t metadata::write(const byte* buffer, size_t size_to_write, off_t offset) {
    auto key = nmfs::structures::utils::data_object_key(path, static_cast<uint32_t>(offset / context.maximum_object_size));
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

        // TODO: Check return value from backend
        context.backend->put(key, offset_in_object, value);
        offset_in_object = 0;
        remain_size_in_object = context.maximum_object_size;
        remain_size_to_write -= size_to_write_in_object;
        buffer += size_to_write_in_object;
    }

    return size_to_write;
}

ssize_t metadata::read(byte* buffer, size_t size_to_read, off_t offset) {
    auto key = nmfs::structures::utils::data_object_key(path, static_cast<uint32_t>(offset / context.maximum_object_size));
    auto offset_in_object = static_cast<uint32_t>(offset % context.maximum_object_size);
    uint32_t remain_size_in_object = context.maximum_object_size - offset_in_object;
    size_t remain_size_to_read = size_to_read;

    while (remain_size_to_read > 0) {
        size_t size_to_read_in_object = std::min<size_t>(remain_size_in_object, remain_size_to_read);
        auto value = borrower_slice(buffer, remain_size_in_object);

        // TODO: Check return value from backend
        context.backend->get(key, offset_in_object, size_to_read_in_object, value);
        offset_in_object = 0;
        remain_size_in_object = context.maximum_object_size;
        remain_size_to_read -= size_to_read_in_object;
        buffer += size_to_read_in_object;
    }

    return size_to_read;
}

void metadata::truncate(off_t new_size) {
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

void metadata::sync() {
    if (dirty) {
        auto key = borrower_slice(path);
        on_disk::metadata on_disk_structure = to_on_disk_structure();
        auto value = borrower_slice(&on_disk_structure, sizeof(on_disk_structure));

        context.backend->put(key, value);
        dirty  = false;
    }
}

void metadata::remove_data_objects(uint32_t index_from, uint32_t index_to) {
    auto key = nmfs::structures::utils::data_object_key(path, index_from);

    for (uint32_t i = index_from; i <= index_to; i++) {
        key.update_index(i);
        context.backend->remove(key);
    }
}
