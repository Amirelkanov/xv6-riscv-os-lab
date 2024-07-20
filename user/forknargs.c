#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "lab2_checkers.c"

const int BUFF_SIZE = 10;

void child_proc(int *p) {
    close(p[1]); // Будем только читать
    char arg_buff[BUFF_SIZE];
    int read_status;

    while ((read_status = read(p[0], arg_buff, BUFF_SIZE)) != 0) {
        check_read_status(read_status);
        printf("%s", arg_buff);
    }

    close(p[0]);
    exit(0);
}

int main(int argc, char **argv) {
    int p[2]; // [rd, wd]
    pipe(p);

    int pid = fork();
    fork_check(pid);

    if (pid == 0) {
        child_proc(p);
    }

    close(p[0]); // Будем только писать
    int arg_len;
    for (int i = 1; i < argc; i++) {
        arg_len = strlen(argv[i]);

        if (arg_len >= BUFF_SIZE) { // Завершаемся, как джентльмены
            close(p[1]);
            wait((int *) 0);
            raise_err("Buffer overflow.\n");
        }

        int arg_write_status = write(p[1], argv[i], strlen(argv[i]));
        check_write_status(arg_write_status, strlen(argv[i]));
        int n_write_status = write(p[1], "\n", 1);
        check_write_status(n_write_status, 1);
    }

    close(p[1]);
    wait((int *) 0);
    exit(0);
}