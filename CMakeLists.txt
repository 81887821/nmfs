cmake_minimum_required(VERSION 3.0)
project(nmfs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(PkgConfig REQUIRED)
find_library(rados librados.so)
pkg_check_modules(FUSE REQUIRED fuse)

add_executable(nmfs main.cpp fuse_ops.cpp)
target_link_libraries(nmfs ${FUSE_LIBRARIES} rados)
target_include_directories(nmfs PUBLIC ${FUSE_INCLUDE_DIRS})
target_compile_options(nmfs PUBLIC ${FUSE_CFLAGS_OTHER})
