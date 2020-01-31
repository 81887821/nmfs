#ifndef NMFS_STRUCTURES_DIRECTORY_HPP
#define NMFS_STRUCTURES_DIRECTORY_HPP

#include <algorithm>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <set>
#include "../fuse.hpp"
#include "metadata.hpp"
#include "../exceptions/is_not_directory.hpp"
#include "../exceptions/file_does_not_exist.hpp"
#include "../exceptions/type_not_supported.hpp"
#include "../logger/log.hpp"

namespace nmfs::structures {

template<typename directory_entry_type>
class directory {
public:
    metadata& directory_metadata;

    explicit inline directory(metadata& metadata);
    inline directory(directory&& other, std::string_view old_directory_path, std::string_view new_directory_path);
    inline ~directory();

    inline void add_file(std::string_view file_name, const metadata& metadata);
    inline void remove_file(std::string_view file_name);
    inline void flush() const;
    inline void fill_buffer(const fuse_directory_filler& filler);
    [[nodiscard]] constexpr size_t number_of_files() const;
    [[nodiscard]] constexpr bool empty() const;
    inline void remove();
    inline const directory_entry_type& get_entry(std::string_view file_name) const;
    inline void move_entry(std::string_view old_path, std::string_view new_path, directory& target_directory);

protected:
    std::set<directory_entry_type> files;
    size_t size;
    mutable std::shared_ptr<std::shared_mutex> mutex;
    mutable bool dirty;

private:
    [[nodiscard]] inline std::unique_ptr<byte[]> serialize() const;
    inline void parse(std::unique_ptr<byte[]> buffer);
};

}

#endif //NMFS_STRUCTURES_DIRECTORY_HPP
