cmake_minimum_required(VERSION 3.0)
project(nmfs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(PkgConfig REQUIRED)
find_library(rados librados.so)
pkg_check_modules(FUSE3 REQUIRED fuse3)
pkg_check_modules(UUID REQUIRED uuid)

add_executable(nmfs
        main.cpp
        main.hpp
        kv_backends/kv_backend.hpp
        kv_backends/rados_backend.cpp
        kv_backends/rados_backend.hpp
        memory_slices/slice.hpp
        memory_slices/owner_slice.hpp
        memory_slices/borrower_slice.hpp
        fuse_operations.cpp
        fuse_operations.hpp
        utils.cpp
        utils.hpp
        primitive_types.hpp
        structures/super_object.hpp
        structures/super_object.impl.hpp
        structures/metadata.hpp
        structures/metadata.impl.hpp
        structures/directory.hpp
        structures/directory.impl.hpp
        structures/directory_entry.hpp
        structures/directory_entry.impl.hpp
        structures/on_disk/super_object.hpp
        structures/on_disk/metadata.hpp
        structures/utils/data_object_key.hpp
        fuse.hpp
        mapper.hpp
        local_caches/caching_policy/all.impl.hpp
        local_caches/caching_policy/all.fwd.hpp
        local_caches/caching_policy/evict_on_last_close.hpp
        local_caches/caching_policy/evict_on_last_close.impl.hpp
        local_caches/cache_store.fwd.hpp
        local_caches/cache_store.hpp
        local_caches/cache_store.impl.hpp
        structures/indexing_types/all.fwd.hpp
        structures/indexing_types/all.hpp
        structures/indexing_types/all.impl.hpp
        structures/indexing_types/full_path/indexing.hpp
        structures/indexing_types/full_path/indexing.impl.hpp
        structures/indexing_types/custom/indexing.hpp
        structures/indexing_types/custom/indexing.impl.hpp
        configuration.hpp
        kv_backends/exceptions/kv_backend_exception.hpp
        kv_backends/exceptions/generic_kv_api_failure.hpp
        kv_backends/exceptions/key_does_not_exist.hpp
        kv_backends/exceptions/backend_initialization_failure.hpp
        exceptions/nmfs_exception.hpp
        exceptions/file_does_not_exist.hpp
        exceptions/file_already_exists.hpp
        exceptions/is_not_directory.hpp
        exceptions/type_not_supported.hpp
        logger/log.hpp
        logger/log_locations.hpp
        logger/log_levels.hpp
        logger/log_stream.hpp
        logger/no_log_stream.hpp
        logger/write_bytes.hpp
        structures/indexing_types/custom/metadata.hpp
        structures/indexing_types/custom/metadata.impl.hpp
        structures/indexing_types/custom/on_disk/metadata.hpp
        structures/indexing_types/custom/directory_entry.hpp
        structures/indexing_types/custom/directory_entry.impl.hpp
        structures/indexing_types/full_path/metadata.hpp
        structures/indexing_types/full_path/metadata.impl.hpp
        structures/indexing_types/full_path/directory_entry.hpp
        structures/indexing_types/full_path/directory_entry.impl.hpp
        local_caches/utils/open_context.hpp
        local_caches/utils/directory_open_context.hpp
        local_caches/utils/no_lock.hpp
        local_caches/caching_policy/hold_closed_cache_for.hpp
        local_caches/caching_policy/hold_closed_cache_for.impl.hpp
        )
target_link_libraries(nmfs ${FUSE3_LIBRARIES} ${UUID_LIBRARIES} rados)
target_include_directories(nmfs PUBLIC ${FUSE3_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS})
target_compile_options(nmfs PUBLIC ${FUSE3_CFLAGS_OTHER} ${UUID_CFLAGS_OTHER})
