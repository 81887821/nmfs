#ifndef NMFS_STRUCTURES_DIRECTORY_IMPL_HPP
#define NMFS_STRUCTURES_DIRECTORY_IMPL_HPP

#include "directory.hpp"
#include "../utils.hpp"

#include "metadata.impl.hpp"

namespace nmfs::structures {

template<typename indexing>
inline directory<indexing>::directory(nmfs::structures::metadata<indexing>& metadata)
    : directory_metadata(metadata),
      size(sizeof(on_disk_size_type)),
      dirty(metadata.size == 0),
      mutex(std::make_shared<std::shared_mutex>()) {
    if (!S_ISDIR(metadata.mode)) {
        throw nmfs::exceptions::is_not_directory();
    } else if (metadata.size > 0) {
        std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(metadata.size);

        metadata.read(buffer.get(), metadata.size, 0);
        parse(std::move(buffer));
    }
}

template<typename indexing>
directory<indexing>::directory(directory&& other, metadata<indexing>& metadata, std::string_view old_directory_path, std::string_view new_directory_path)
    : directory_metadata(metadata),
      /* Moving files and size is deferred until moving children is finished */
      dirty(false),
      mutex(std::make_shared<std::shared_mutex>()) {
    auto other_shared_lock = std::shared_lock(*other.mutex);
    auto this_unique_lock = std::unique_lock(*mutex);

    other.dirty = false;
    other.directory_metadata.valid = false;

    for (const auto& file: other.files) {
        std::string old_path = std::string(old_directory_path) + path_delimiter + file.file_name;
        std::string new_path = std::string(new_directory_path) + path_delimiter + file.file_name;
        log::information(log_locations::directory_operation) << "Moving child entry (old_path = " << old_path << ", new_path = " << new_path << ")\n";
        mode_t type = directory_metadata.context.cache->get_type(old_path);

        if (S_ISDIR(type)) {
            directory_metadata.context.cache->move_directory(old_path, new_path);
        } else if (S_ISREG(type)) {
            directory_metadata.context.cache->move(old_path, new_path);
        } else {
            throw nmfs::exceptions::type_not_supported(type);
        }
    }

    other_shared_lock.unlock();
    auto other_unique_lock = std::unique_lock(*other.mutex);

    files = std::move(other.files);
    size = other.size;
    other.size = 0;
    dirty = other.dirty;
}

template<typename indexing>
inline directory<indexing>::~directory() {
    flush();
}

template<typename indexing>
inline void directory<indexing>::add_file(std::string_view file_name, const metadata<indexing>& metadata) {
    log::information(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << "(file_name = " << file_name << ")\n";

    auto content = directory_entry_type(std::string(file_name), metadata);

    auto result = files.emplace(std::move(content));
    if (result.second) {
        size += result.first->size();
        dirty = true;
    } else {
        log::warning(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << " failed: files.emplace returned false";
    }
}

template<typename indexing>
inline void directory<indexing>::remove_file(std::string_view file_name) {
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

template<typename indexing>
inline void directory<indexing>::flush() const {
    log::information(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << "()\n";

    if (dirty) {
        if (size < directory_metadata.size) {
            directory_metadata.truncate(size);
        }
        directory_metadata.write(serialize().get(), size, 0);
        directory_metadata.flush();
        dirty = false;
    } else if (directory_metadata.dirty) {
        directory_metadata.flush();
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
        index += file.serialize(&buffer[index]);
    }

    return buffer;
}

template<typename indexing>
inline void directory<indexing>::parse(std::unique_ptr<byte[]> buffer) {
    const byte* current = buffer.get();

    on_disk_size_type number_of_files = *reinterpret_cast<const on_disk_size_type*>(current);
    current += sizeof(on_disk_size_type);

    for (on_disk_size_type i = 0; i < number_of_files; i++) {
        auto content = directory_entry_type(&current);
        files.emplace(std::move(content));
    }

    size = current - buffer.get();
}

template<typename indexing>
void directory<indexing>::fill_buffer(const fuse_directory_filler& filler) {
    for (const auto& content: files) {
        content.fill(filler);
    }
}

template<typename indexing>
constexpr size_t directory<indexing>::number_of_files() const {
    auto lock = std::shared_lock(*mutex);
    return files.size();
}

template<typename indexing>
constexpr bool directory<indexing>::empty() const {
    return files.empty();
}

template<typename indexing>
void directory<indexing>::remove() {
    log::information(log_locations::directory_operation) << std::hex << std::showbase << "(" << &directory_metadata << ") " << __func__ << "()\n";
    auto lock = std::unique_lock(*mutex);

    directory_metadata.remove();
    dirty = false;
}

template<typename indexing>
const typename directory<indexing>::directory_entry_type& directory<indexing>::get_entry(std::string_view file_name) const {
    auto iterator = std::find_if(files.begin(), files.end(), directory_entry_type::find_by_name(file_name));

    if (iterator != files.end()) {
        return *iterator;
    } else {
        throw nmfs::exceptions::file_does_not_exist(file_name);
    }
}

template<typename indexing>
void directory<indexing>::move_entry(std::string_view old_path, std::string_view new_path, directory& target_directory) {
    std::string_view old_file_name = get_filename(old_path);
    std::string_view new_file_name = get_filename(new_path);
    auto iterator = std::find_if(files.begin(), files.end(), directory_entry_type::find_by_name(old_file_name));

    if (iterator != files.end()) {
        size_t entry_size = iterator->size();
        auto entry = files.extract(iterator);
        size -= entry_size;
        dirty = true;

        entry.value().file_name = std::string(new_file_name);
        entry_size = entry.value().size();

        auto insert_result = target_directory.files.insert(std::move(entry));
        target_directory.size += entry_size;
        target_directory.dirty = true;
    } else {
        throw nmfs::exceptions::file_does_not_exist(old_path);
    }
}

}

#endif //NMFS_STRUCTURES_DIRECTORY_IMPL_HPP
