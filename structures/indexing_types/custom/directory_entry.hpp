#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_HPP

#include "../../directory_entry.hpp"
#include "metadata.hpp"

namespace nmfs::structures::indexing_types::custom {

class directory_entry: public nmfs::structures::directory_entry<indexing> {
public:
    uuid_t uuid{};
    mode_t type;

    inline directory_entry(std::string file_name, const nmfs::structures::metadata<indexing>& metadata);
    inline explicit directory_entry(const byte** buffer);

    inline on_disk_size_type serialize(byte* buffer) const override;
    [[nodiscard]] inline size_t size() const override;
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_DIRECTORY_ENTRY_HPP
