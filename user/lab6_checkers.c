#ifndef XV6_RISCV_OS_LAB6_CHECKERS_C
#define XV6_RISCV_OS_LAB6_CHECKERS_C

#include "check_helpers.c"

void fork_check(int pid, char *buff_needs_to_be_freed) {
    if (pid < 0) {
        free(buff_needs_to_be_freed);
        raise_err("Fork error.\n");
    }
}

void pipe_check(int pipe_code, char *buff_needs_to_be_freed) {
    if (pipe_code < 0) {
        free(buff_needs_to_be_freed);
        raise_err("Pipe error.\n");
    }
}

void kill_check(int kill_status, char *buff_needs_to_be_freed) {
    if (kill_status < 0) {
        free(buff_needs_to_be_freed);
        raise_err("THIS CHILD IS IMMORTAL!!!\n");
    }
}

void check_read_status(int read_status, char *buff_needs_to_be_freed) {
    if (read_status == -1) {
        free(buff_needs_to_be_freed);
        raise_err("Error occurred while reading input.\n");
    }
}

void check_write_status(int write_status, int bytes_to_write, char *buff_needs_to_be_freed) {
    if (write_status != bytes_to_write) {
        free(buff_needs_to_be_freed);
        raise_err("Write error.\n");
    }
}

#endif