#ifndef NMFS_STRUCTURES_DIRECTORY_ENTRY_HPP
#define NMFS_STRUCTURES_DIRECTORY_ENTRY_HPP

#include <algorithm>
#include <string>
#include "../primitive_types.hpp"
#include "../fuse.hpp"
#include "metadata.hpp"

namespace nmfs::structures {

template<typename indexing>
class directory_entry {
public:
    std::string file_name;

    inline directory_entry(std::string file_name, const metadata<indexing>& metadata);
    inline explicit directory_entry(const byte** buffer);

    inline virtual on_disk_size_type serialize(byte* buffer) const;
    [[nodiscard]] inline virtual size_t size() const;
    inline virtual void fill(const fuse_directory_filler& filler) const;

    inline virtual bool operator<(const directory_entry& other) const;
    inline virtual bool operator==(const directory_entry& other) const;

    inline static std::function<bool(const directory_entry&)> find_by_name(std::string_view file_name);
};

}

#endif //NMFS_STRUCTURES_DIRECTORY_ENTRY_HPP
