#ifndef NMFS__FUSE_HPP
#define NMFS__FUSE_HPP

#define FUSE_USE_VERSION 35
#include <fuse.h>

namespace nmfs {

/**
 * A wrapper class of fuse_fill_dir_t
 */
class fuse_directory_filler {
public:
    /**
     * The kernel wants to prefill the inode cache during readdir.
     * This flag can be ignored.
     */
    bool prefill_file_stat;

    constexpr fuse_directory_filler(void* buffer, fuse_fill_dir_t filler, fuse_readdir_flags flags);

    constexpr int operator()(const char* name, const struct stat* stat) const;
    constexpr int operator()(const char* name) const;

private:
    void* buffer;
    fuse_fill_dir_t filler;
};

constexpr fuse_directory_filler::fuse_directory_filler(void* buffer, fuse_fill_dir_t filler, fuse_readdir_flags flags)
    : prefill_file_stat(flags & FUSE_READDIR_PLUS),
      buffer(buffer),
      filler(filler) {
}

constexpr int fuse_directory_filler::operator()(const char* name, const struct stat* stat) const {
    return filler(buffer, name, stat, 0, FUSE_FILL_DIR_PLUS);
}

constexpr int fuse_directory_filler::operator()(const char* name) const {
    return filler(buffer, name, nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
}

}

#endif //NMFS__FUSE_HPP
