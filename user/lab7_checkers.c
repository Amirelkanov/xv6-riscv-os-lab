#ifndef XV6_RISCV_OS_LAB7_CHECKERS_C
#define XV6_RISCV_OS_LAB7_CHECKERS_C

#include "check_helpers.c"
#include "kernel/fcntl.h"

void safe_mkdir(const char *name)
{
    myassert(mkdir(name) == 0, "Mkdir error.\n");
}

void safe_chdir(const char *name)
{
    myassert(chdir(name) == 0, "Chdir error.\n");
}

void safe_create_file(const char *name, char content)
{
    int fd = open(name, O_CREATE | O_WRONLY);
    myassert(fd >= 0, "Open error.\n");
    myassert(write(fd, &content, 1) == 1, "Write error.\n");
    myassert(close(fd) == 0, "Close error.\n");
}

void safe_symlink(const char *target, const char *name)
{
    myassert(symlink(target, name) == 0, "Symlink error.\n");
}

void safe_unlink(const char *name)
{
    myassert(unlink(name) == 0, "Unlink error.\n");
}

void check(const char *target, const char *symlink, int is_err)
{
    int target_fd, linked_fd;
    char symlink_content, target_content;

    linked_fd = open(symlink, O_RDONLY);
    myassert((linked_fd >= 0 && !is_err) || (linked_fd < 0 && is_err), "Open error.\n");

    target_fd = open(target, O_RDONLY);
    myassert((target_fd >= 0 && !is_err) || (target_fd < 0 && is_err), "Open error.\n");

    
    if (linked_fd >= 0 && target_fd >= 0)
    {
        myassert(read(target_fd, &target_content, 1) == 1, "Read error.\n");
        myassert(read(linked_fd, &symlink_content, 1) == 1, "Read error.\n");

        myassert(symlink_content == target_content, "Content mismatch.\n");

        myassert(close(target_fd) == 0, "Close error.\n");
        myassert(close(linked_fd) == 0, "Close error.\n");
    }
}

#endif