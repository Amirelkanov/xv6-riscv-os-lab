#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "lab8_checkers.c"
#include "kernel/fcntl.h"

#define BUFSIZE 4096
#define BUFSIZE_IN_BYTES (BUFSIZE * sizeof(uint64))

uint64 rand_next(uint64 elem) { // 2^64 - mod
    return elem * 6364136223846793005 + 1;
}

uint64 get_rw_bytes(uint64 i) {
    return (i * sizeof(uint64) < BUFSIZE_IN_BYTES) ? i * sizeof(uint64) : BUFSIZE_IN_BYTES;
}

// bigfile [SEED] [MODE: -x(tra), -b(ig), -m(edium), -s(mall), -c(ustom)]
// При custom следует 3 аргумент: размер файла в байтах
int main(int argc, char **argv) {
    char *filename = "bigfile.txt";
    int filesize, fd, return_code = 0;
    uint64 seed, rand_int, i;

    check_args_n_set_filesize(&seed, &filesize, argc, argv);
    uint64 *buff = (uint64 *) malloc(BUFSIZE_IN_BYTES);
    check_malloc(buff);

    fd = open(filename, O_CREATE | O_WRONLY | O_TRUNC);
    check_open(fd, 0);

    filesize /= sizeof(uint64);

    rand_int = seed;
    i = filesize;
    while (i > 0) {
        uint64 bytes_to_write = get_rw_bytes(i);

        for (int j = 0; j < bytes_to_write / sizeof(uint64); j++) {
            buff[j] = rand_int;
            rand_int = rand_next(rand_int);
        }

        if (write(fd, buff, bytes_to_write) != bytes_to_write) {
            fprintf(2, "Error while writing.\n");
            return_code = 1;
            goto end;
        }
        i -= (bytes_to_write / sizeof(uint64));
    }
    close(fd);

    fd = open(filename, O_RDONLY);
    check_open(fd, buff);

    rand_int = seed;
    i = filesize;
    while (i > 0) {
        uint64 bytes_to_read = get_rw_bytes(i);
        if (read(fd, buff, bytes_to_read) != bytes_to_read) {
            fprintf(2, "Error while reading.\n");
            return_code = 1;
            goto end;
        }

        for (int j = 0; j < bytes_to_read / sizeof(uint64); j++) {
            if (rand_int != buff[j]) {
                fprintf(2, "Mismatch. Expected: %llu, Got: %llu.\n", rand_int, buff[j]);
                return_code = 2;
                goto end;
            }
            rand_int = rand_next(rand_int);
        }

        i -= bytes_to_read / sizeof(uint64);
    }

    printf("Everything done perfectly!\nCreated temp file of %d uint64 numbers generated randomly from seed=%d\n",
           filesize, seed);

    end:
    close(fd);
    free(buff);
    unlink(filename);
    exit(return_code);
}