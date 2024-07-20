#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/riscv.h"
#include "kernel/dmsgbuff.h"
#include "user/check_helpers.c"

enum EVENT_CLASS get_event_class_by_char(char ev_class_chr) {
  switch (ev_class_chr) {
    case 'c':
      return SYSCALL;
    case 'i':
      return INTERRUPT;
    case 's':
      return SWITCH;
    case 'e':
      return EXEC;
    default:
      fprintf(STDERR_D, "Unknown class: %c.\n", ev_class_chr);
      exit(1);
  }
}

/* Изменяет поведения средства протоколирования событий:
 *  Без указания флагов утилита включает/отключает соответствующие классы логирования:
        chlog [-/+][EVENT CLASS 1] [-/+][EVENT CLASS 2] ...,
    где event class'ы записываются следующим образом: c - SYSCALL, i - INTERRUPT, s - SWITCH, e - EXEC.

    Пример:
        chlog +c +s +i -e
    включит протоколирование событий SYSCALL, SWITCH, INTERRUPT и отключит протоколирование события EXEC

 *  С указанием флага -t утилита меняет промежуток протоколирования на указанное количество тиков:
        chlog -t [AMOUNT OF TICKS]
*/
int main(int argc, char **argv) {
  if (argc < 2) raise_err("Invalid number of arguments.\n");
  set_stop_ticks(100);
  if (!strcmp(argv[1], "-t")) {
    set_stop_ticks(s_atoi(argv[2]));
  } else {
    for (int i = 1; i < argc; i++) {
      if (strlen(argv[i]) != 2) raise_err("Invalid argument.\n");
      toggle_class_log(get_event_class_by_char(argv[i][1]), argv[i][0] == '+');
    }
  }
  exit(0);
}