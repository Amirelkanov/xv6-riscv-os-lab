#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"
#include "kernel/dmsgbuff.h"
#include "user/check_helpers.c"

void test_correct_dmesg(void) {
    printf("[test_correct_dmesg] ");
    char *user_buff = malloc(DIAG_MSG_BUFF_SIZE + 1);
    myassert(dmesg((uint64) user_buff, DIAG_MSG_BUFF_SIZE + 1) == 0, "Correct test failed.\n");
    printf("OK.\nOutput of Diagnostic message buffer:\n%s\n", user_buff);
    free(user_buff);
}

void test_small_user_buff(void) {
    printf("[test_small_user_buff] ");
    char *user_buff = malloc(1);
    myassert(dmesg((uint64) user_buff, 1) == -2, "Small user buffer test failed.\n");
    printf("OK\n");
    free(user_buff);
}

void test_null_buff_pointer(void) {
    printf("[test_null_buff_pointer] ");
    myassert(dmesg(0, DIAG_MSG_BUFF_SIZE + 1) == -1, "Null user buffer test failed.\n");
    printf("OK\n");
}

int main() {
    for (enum EVENT_CLASS curr_class = SYSCALL; curr_class < NEVENTCLASSES; curr_class++) {
        toggle_class_log(curr_class, 1);
    }
    set_stop_ticks(100);

    // Тут я не все случаи покрыл: copyout может ныть еще в 3 местах, но чисто для быстрой проверки сойдет
    printf("===== Dmesg tests started =====\n");
    test_correct_dmesg();
    test_small_user_buff();
    test_null_buff_pointer();
    printf("===== All tests passed successfully! =====\n");
    exit(0);
}