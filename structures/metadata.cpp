#include <utility>
#include "metadata.hpp"
#include "../memory_slices/borrower_slice.hpp"

using namespace nmfs::structures;

metadata::metadata(super_object& super, std::string path, fuse_context* fuse_context, mode_t mode)
    : context(super),
      path(std::move(path)),
      link_count(1),
      owner(fuse_context->uid),
      group(fuse_context->gid),
      mode(mode),
      size(0) {
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

ssize_t metadata::write(const byte* buffer, size_t size, off_t offset) {
    // TODO
    throw std::runtime_error("Not implemented");
}

ssize_t metadata::read(byte* buffer, size_t size, off_t offset) {
    // TODO
    throw std::runtime_error("Not implemented");
}

void metadata::truncate(off_t length) {
    // TODO
    throw std::runtime_error("Not implemented");
}

void metadata::sync_metadata() {
    auto key = borrower_slice(path);
    on_disk::metadata on_disk_structure = to_on_disk_structure();
    auto value = borrower_slice(&on_disk_structure, sizeof(on_disk_structure));

    context.backend->put(key, value);
}
