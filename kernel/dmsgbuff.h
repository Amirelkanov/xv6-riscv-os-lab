#ifndef XV6_RISCV_OS_LAB1_DMSGBUFF_H
#define XV6_RISCV_OS_LAB1_DMSGBUFF_H

#include "spinlock.h"
#include "param.h"

typedef struct {
    struct spinlock lock;
    char buff[NDMSGPAGE * PGSIZE];
    int head, tail;
} dmsg_buff_t;

typedef struct {
    struct spinlock lock;
    int stop_ticks;
    int enabled_classes[NEVENTCLASSES]; // Вообще, было бы круто bool, но такого чуда тут нет
} ev_classes_logger_t;

enum EVENT_CLASS {
    SYSCALL, INTERRUPT, SWITCH, EXEC
};

#endif
