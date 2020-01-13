#include "main.hpp"
#include "fuse_operations.hpp"

int main(int argc, char* argv[]) {
    fuse_operations fops;

    //fops = nmfs::fuse_operations::get_fuse_ops();

    return fuse_main(argc, argv, &fops, NULL);
}
