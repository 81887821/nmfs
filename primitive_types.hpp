#ifndef NMFS_PRIMITIVE_TYPES_HPP
#define NMFS_PRIMITIVE_TYPES_HPP

namespace nmfs {

typedef char byte;
static_assert(sizeof(byte) == 1);

using on_disk_size_type = uint32_t;

}

#endif //NMFS_PRIMITIVE_TYPES_HPP
