#ifndef XV6_RISCV_OS_LAB8_CHECKERS_C
#define XV6_RISCV_OS_LAB8_CHECKERS_C

#include "kernel/fs.h"
#include "check_helpers.c"

#define MAXSIZE (11 + 256 + 256 * 256) * BSIZE

void check_args_n_set_filesize(uint64 *seed, int *filesize, int argc, char **argv) {
    if (argc < 2) raise_err("Not enough arguments.\n");

    *seed = s_atoi(argv[1]);
    if (!strcmp(argv[2], "-x")) *filesize = MAXSIZE;
    else if (!strcmp(argv[2], "-b")) *filesize = MAXSIZE / 8;
    else if (!strcmp(argv[2], "-m")) *filesize = MAXSIZE / 1024;
    else if (!strcmp(argv[2], "-s")) *filesize = 8;
    else if (!strcmp(argv[2], "-c")) {
        if (argc != 4) raise_err("Not enough arguments.\n");
        int size = s_atoi(argv[3]);
        if (size > MAXSIZE) raise_err("Invalid custom size.\n");
        *filesize = size;
    } else raise_err("Unknown argument.\n");
}

void check_malloc(void *ptr) {
    if (ptr == 0) raise_err("Malloc error.\n");
}

void check_open(int fd, void* mem_needs_to_be_freed) {
    if (fd < 0) {
        free(mem_needs_to_be_freed);
        raise_err("Error when opening file.\n");
    }
}

#endif