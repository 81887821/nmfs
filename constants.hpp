#ifndef NMFS_CONSTANTS_HPP
#define NMFS_CONSTANTS_HPP

#include <rados/librados.hpp>

namespace nmfs {

/*
 * mode 1 : full path
 * mode 2 : UNIX-like
 * mode 3 : custom
 */
const uint8_t key_mode = 1;

}

#endif
