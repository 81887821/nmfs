cmake_minimum_required(VERSION 3.0)
project(nmfs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(PkgConfig REQUIRED)
find_library(rados librados.so)
pkg_check_modules(FUSE3 REQUIRED fuse3)

add_executable(nmfs main.cpp main.hpp constants.hpp kv_backends/kv_backend.hpp memory_slices/slice.hpp kv_backends/rados_backend.cpp kv_backends/rados_backend.hpp memory_slices/owner_slice.hpp memory_slices/borrower_slice.hpp fuse_operations.cpp fuse_operations.hpp utils.cpp utils.hpp primitive_types.hpp structures/super_object.cpp structures/super_object.hpp structures/metadata.cpp structures/metadata.hpp structures/directory.cpp structures/directory.hpp structures/on_disk/super_object.hpp structures/on_disk/metadata.hpp fuse.hpp structures/utils/data_object_key.hpp mapper.hpp)
target_link_libraries(nmfs ${FUSE3_LIBRARIES} rados)
target_include_directories(nmfs PUBLIC ${FUSE3_INCLUDE_DIRS})
target_compile_options(nmfs PUBLIC ${FUSE3_CFLAGS_OTHER})
