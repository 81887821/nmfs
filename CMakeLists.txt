cmake_minimum_required(VERSION 3.0)
project(nmfs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE3 REQUIRED fuse3)

add_executable(nmfs main.cpp)
target_link_libraries(nmfs ${FUSE3_LIBRARIES})
target_include_directories(nmfs PUBLIC ${FUSE3_INCLUDE_DIRS})
target_compile_options(nmfs PUBLIC ${FUSE3_CFLAGS_OTHER})
