#ifndef NMFS_STRUCTURES_DIRECTORY_HPP
#define NMFS_STRUCTURES_DIRECTORY_HPP

#include <algorithm>
#include <memory>
#include <string>
#include <set>
#include "../fuse.hpp"
#include "metadata.hpp"
#include "../exceptions/is_not_directory.hpp"

namespace nmfs::structures {

template<typename indexing>
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

private:
    std::set<typename indexing::directory_content_type> files;
    size_t size;
    mutable bool dirty;

    [[nodiscard]] inline std::unique_ptr<byte[]> serialize() const;
    inline void parse(std::unique_ptr<byte[]> buffer);
};

template<typename indexing>
inline directory<indexing>::directory(nmfs::structures::metadata& metadata)
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

template<typename indexing>
inline directory<indexing>::~directory() {
    flush();
}

template<typename indexing>
inline void directory<indexing>::add_file(std::string_view file_name, const metadata& metadata) {
    auto content = indexing::to_directory_content(file_name, metadata);
    size_t content_size = indexing::get_content_size(content);

    files.emplace(std::move(content));
    size += content_size;
    dirty = true;
}

template<typename indexing>
inline void directory<indexing>::remove_file(std::string_view file_name) {
    auto iterator = std::find_if(files.begin(), files.end(), indexing::content_finder_by_name(file_name));

    if (iterator != files.end()) {
        files.erase(iterator);
    }
}

template<typename indexing>
inline void directory<indexing>::flush() const {
    if (dirty) {
        if (size < directory_metadata.size) {
            directory_metadata.truncate(size);
        }
        directory_metadata.write(serialize().get(), size, 0);
        directory_metadata.flush();
        dirty = false;
    }
}

template<typename indexing>
inline std::unique_ptr<byte[]> directory<indexing>::serialize() const {
    on_disk_size_type number_of_files = files.size();
    std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(size);
    size_t index = 0;

    *reinterpret_cast<on_disk_size_type*>(&buffer[index]) = number_of_files;
    index += sizeof(number_of_files);

    for (const auto& file: files) {
        index += indexing::serialize_directory_content(&buffer[index], file);
    }

    return buffer;
}

template<typename indexing>
inline void directory<indexing>::parse(std::unique_ptr<byte[]> buffer) {
    size_t index = 0;

    on_disk_size_type number_of_files = *reinterpret_cast<on_disk_size_type*>(&buffer[index]);
    index += sizeof(on_disk_size_type);

    typename indexing::directory_content_type content;
    on_disk_size_type parsed_size;

    for (on_disk_size_type i = 0; i < number_of_files; i++) {
        std::tie(content, parsed_size) = indexing::parse_directory_content(&buffer[index]);
        files.emplace(std::move(content));
        index += parsed_size;
    }

    size = index;
}

template<typename indexing>
void directory<indexing>::fill_buffer(const fuse_directory_filler& filler) {
    for (const auto& content: files) {
        indexing::fill_content(content, filler);
    }
}

template<typename indexing>
constexpr size_t directory<indexing>::number_of_files() const {
    return files.size();
}

template<typename indexing>
void directory<indexing>::remove() {
    directory_metadata.remove();
    dirty = false;
}

}

#endif //NMFS_STRUCTURES_DIRECTORY_HPP
