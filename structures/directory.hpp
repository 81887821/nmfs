#ifndef NMFS_STRUCTURES_DIRECTORY_HPP
#define NMFS_STRUCTURES_DIRECTORY_HPP

#include <algorithm>
#include <memory>
#include <string>
#include <set>
#include "../fuse.hpp"
#include "metadata.hpp"
#include "../exceptions/is_not_directory.hpp"
#include "../logger/log.hpp"

namespace nmfs::structures {

template<typename directory_entry_type>
class directory {
public:
    metadata& directory_metadata;

    explicit inline directory(metadata& metadata);
    inline ~directory();

    inline void add_file(std::string_view file_name, const metadata& metadata);
    inline void remove_file(std::string_view file_name);
    inline void flush() const;
    inline void fill_buffer(const fuse_directory_filler& filler);
    [[nodiscard]] constexpr size_t number_of_files() const;
    inline void remove();

protected:
    std::set<directory_entry_type> files;
    size_t size;
    mutable bool dirty;

private:
    [[nodiscard]] inline std::unique_ptr<byte[]> serialize() const;
    inline void parse(std::unique_ptr<byte[]> buffer);
};

template<typename directory_entry_type>
inline directory<directory_entry_type>::directory(nmfs::structures::metadata& metadata)
    : directory_metadata(metadata),
      size(sizeof(on_disk_size_type)),
      dirty(metadata.size == 0) {
    if (!S_ISDIR(metadata.mode)) {
        throw nmfs::exceptions::is_not_directory();
    } else if (metadata.size > 0) {
        std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(metadata.size);

        metadata.read(buffer.get(), metadata.size, 0);
        parse(std::move(buffer));
    }
}

template<typename directory_entry_type>
inline directory<directory_entry_type>::~directory() {
    flush();
}

template<typename directory_entry_type>
inline void directory<directory_entry_type>::add_file(std::string_view file_name, const metadata& metadata) {
    log::information(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << "(file_name = " << file_name << ")\n";

    auto content = directory_entry_type(std::string(file_name), metadata);

    auto result = files.emplace(std::move(content));
    if (result.second) {
        size += content.size();
        dirty = true;
    } else {
        log::warning(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << " failed: files.emplace returned false";
    }
}

template<typename directory_entry_type>
inline void directory<directory_entry_type>::remove_file(std::string_view file_name) {
    log::information(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << "(file_name = " << file_name << ")\n";

    auto iterator = std::find_if(files.begin(), files.end(), directory_entry_type::find_by_name(file_name));

    if (iterator != files.end()) {
        files.erase(iterator);
        size -= iterator->size();
        dirty = true;
    } else {
        log::warning(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << " failed: find_if returned files.end()";
    }
}

template<typename directory_entry_type>
inline void directory<directory_entry_type>::flush() const {
    log::information(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << "()\n";

    if (dirty) {
        if (size < directory_metadata.size) {
            directory_metadata.truncate(size);
        }
        directory_metadata.write(serialize().get(), size, 0);
        directory_metadata.flush();
        dirty = false;
    }
}

template<typename directory_entry_type>
inline std::unique_ptr<byte[]> directory<directory_entry_type>::serialize() const {
    on_disk_size_type number_of_files = files.size();
    std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(size);
    size_t index = 0;

    *reinterpret_cast<on_disk_size_type*>(&buffer[index]) = number_of_files;
    index += sizeof(number_of_files);

    for (const auto& file: files) {
        index += file.serialize(&buffer[index]);
    }

    return buffer;
}

template<typename directory_entry_type>
inline void directory<directory_entry_type>::parse(std::unique_ptr<byte[]> buffer) {
    const byte* current = buffer.get();

    on_disk_size_type number_of_files = *reinterpret_cast<const on_disk_size_type*>(current);
    current += sizeof(on_disk_size_type);

    for (on_disk_size_type i = 0; i < number_of_files; i++) {
        auto content = directory_entry_type(&current);
        files.emplace(std::move(content));
    }

    size = current - buffer.get();
}

template<typename directory_entry_type>
void directory<directory_entry_type>::fill_buffer(const fuse_directory_filler& filler) {
    for (const auto& content: files) {
        content.fill(filler);
    }
}

template<typename directory_entry_type>
constexpr size_t directory<directory_entry_type>::number_of_files() const {
    return files.size();
}

template<typename directory_entry_type>
void directory<directory_entry_type>::remove() {
    log::information(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << "()\n";

    directory_metadata.remove();
    dirty = false;
}

}

#endif //NMFS_STRUCTURES_DIRECTORY_HPP
