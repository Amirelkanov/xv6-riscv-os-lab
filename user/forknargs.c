#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"
#include "user/check_helpers.c"
#include "kernel/dmsgbuff.h"
#include "lab6_checkers.c"

const int BUFF_SIZE = 10;

void child_proc(int *p) {
    close(p[1]); // Будем только читать
    char arg_buff[BUFF_SIZE];
    int read_status;

    while ((read_status = read(p[0], arg_buff, BUFF_SIZE)) != 0) {
        check_read_status(read_status, 0);
        // printf("%s", arg_buff);
    }

    close(p[0]);
    exit(0);
}

void on_all_event_classes_logging() {
    for (enum EVENT_CLASS curr_class = SYSCALL; curr_class < NEVENTCLASSES; curr_class++) {
        toggle_class_log(curr_class, 1);
    }
}


int main(int argc, char **argv) {
    char *user_buff = malloc(DIAG_MSG_BUFF_SIZE + 1);
    on_all_event_classes_logging();
    set_stop_ticks(100);

    int p[2]; // [rd, wd]
    pipe(p);

    int pid = fork();
    fork_check(pid, user_buff);

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
            free(user_buff);
            raise_err("Buffer overflow.\n");
        }

        int arg_write_status = write(p[1], argv[i], strlen(argv[i]));
        check_write_status(arg_write_status, strlen(argv[i]), user_buff);
        int n_write_status = write(p[1], "\n", 1);
        check_write_status(n_write_status, 1, user_buff);
    }

    close(p[1]);
    wait((int *) 0);

    if (dmesg((uint64) user_buff, DIAG_MSG_BUFF_SIZE + 1) < 0) {
        free(user_buff);
        raise_err("Somehow dmesg err occurred.");
    }

    printf("%s", user_buff);
    free(user_buff);

    exit(0);
}