#ifndef NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_HPP
#define NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_HPP

#include <functional>
#include <string_view>
#include "../super_object.hpp"
#include "../../utils.hpp"
#include "../../memory_slices/borrower_slice.hpp"
#include "../../local_caches/cache_store.hpp"

namespace nmfs::indexing_types {
using namespace nmfs::structures;

class custom {
public:
    using slice_type = nmfs::borrower_slice;
    using directory_content_type = std::string;

    static inline slice_type make_directory_key(super_object& context, std::string_view path) {
        DECLARE_CONST_BORROWER_SLICE(slice, path.data(), path.size());
        return slice;
    }

    static inline slice_type make_regular_file_key(super_object& context, std::string_view path) {
        std::string file_data_key; //parent_directory_uuid + file_uuid;
        char* uuid_sum = (char *)malloc(sizeof(char) * file_data_key.size());
        memcpy(uuid_sum, file_data_key.data(), file_data_key.size());
        /*free so erased
        char *uuid_sum = (char *)malloc(sizeof(char) * 72);
        strncpy(uuid_sum, parent_directory_uuid.c_str(), parent_directory_uuid.size());
        strcat(uuid_sum, file_uuid.c_str());
        */
        DECLARE_CONST_BORROWER_SLICE(slice, uuid_sum, file_data_key.size());
        log::information(log_locations::other) << __func__ <<  " created key = " << slice.to_string_view() << "slice size = "<< slice.to_string().length()<<"\n";
        return slice;
    }

    static inline on_disk_size_type serialize_directory_content(byte* buffer, const directory_content_type& content) {
        log::information(log_locations::other) << __func__ << "()\n";
        on_disk_size_type file_name_length = content.size();

        *reinterpret_cast<on_disk_size_type*>(buffer) = file_name_length;
        std::copy(content.begin(), content.end(), &buffer[sizeof(on_disk_size_type)]);

        return sizeof(on_disk_size_type) + file_name_length;
    }

    static inline std::tuple<directory_content_type, on_disk_size_type> parse_directory_content(byte* buffer) {
        log::information(log_locations::other) << __func__ << "()\n";
        on_disk_size_type file_name_length = *reinterpret_cast<on_disk_size_type*>(buffer);
        auto file_name = std::string(&buffer[sizeof(on_disk_size_type)], file_name_length);

        return std::make_tuple(std::move(file_name), sizeof(on_disk_size_type) + file_name_length);
    }

    static inline size_t get_content_size(const directory_content_type& content) {
        log::information(log_locations::other) << __func__ << "()\n";
        return sizeof(on_disk_size_type) + content.length();
    }

    static inline int fill_content(const directory_content_type& content, const fuse_directory_filler& filler) {
        log::information(log_locations::other) << __func__ << "()\n";
        return filler(content.c_str());
    }

    static inline directory_content_type to_directory_content(std::string_view file_name, const metadata& metadata) {
        log::information(log_locations::other) << __func__ << "()\n";
        return std::string(file_name);
    }

    static inline std::function<bool(const directory_content_type&)> content_finder_by_name(std::string_view file_name) {
        log::information(log_locations::other) << __func__ << "()\n";
        return [file_name] (const directory_content_type& other) {
            return file_name == other;
        };
    }
};

}

#endif //NMFS_STRUCTURES_INDEXING_TYPES_CUSTOM_HPP
