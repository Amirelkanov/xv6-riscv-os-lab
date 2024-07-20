#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "dmsgbuff.h"

dmsg_buff_t dmsg_buff;
ev_classes_logger_t ev_classes_logger;

char digits[] = "0123456789abcdef";

void dmsg_buff_init(void) {
    dmsg_buff.head = dmsg_buff.tail = 0;
    initlock(&dmsg_buff.lock, "Diag_msg_buff_lock");
}

void ev_classes_log_init(void) {
    initlock(&ev_classes_logger.lock, "ev_classes_logger_lock");
    memset(ev_classes_logger.enabled_classes, 0, NEVENTCLASSES * sizeof(short));
}

void putb(char b) { // Считаем, что у нас нормальная система и чар весит 1 байт
    dmsg_buff.buff[dmsg_buff.tail % DIAG_MSG_BUFF_SIZE] = b;
    if (dmsg_buff.tail - dmsg_buff.head == DIAG_MSG_BUFF_SIZE) {
        dmsg_buff.head++;
        dmsg_buff.head %= DIAG_MSG_BUFF_SIZE;
        dmsg_buff.tail = dmsg_buff.head + DIAG_MSG_BUFF_SIZE;
    } else {
        dmsg_buff.tail++;
    }
}

void pr_msg_int(int xx, int base, int sign) {
    char buf[16];
    uint x;

    x = (sign && (sign = xx < 0)) ? -xx : xx;

    int i = 0;
    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign)
        buf[i++] = '-';

    while (--i >= 0)
        putb(buf[i]);
}

void pr_msg_ptr(uint64 x) {
    int i;
    putb('0');
    putb('x');
    for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
        putb(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

int pr_msg(enum EVENT_CLASS event_class, const char *fmt, ...) {
    if (fmt == 0)
        panic("null fmt");

    int curr_ticks = ticks;

    acquire(&ev_classes_logger.lock);
    if (ev_classes_logger.enabled_classes[event_class] == 0) {
        release(&ev_classes_logger.lock);
        return -2;
    }
    if (ev_classes_logger.stop_ticks < curr_ticks) {
        release(&ev_classes_logger.lock);
        return -3;
    }
    release(&ev_classes_logger.lock);

    va_list ap;
    int i, c;
    char *s;
    acquire(&dmsg_buff.lock);

    putb('[');
    pr_msg_int(curr_ticks, 10, 1);
    putb(']');
    putb(' ');

    va_start(ap, fmt);
    for (i = 0; (c = fmt[i]) != 0; i++) {
        if (c != '%') {
            putb(c);
            continue;
        }
        c = fmt[++i];
        if (c == 0)
            break;
        switch (c) {
            case 'd':
                pr_msg_int(va_arg(ap,
                int), 10, 1);
                break;
            case 'x':
                pr_msg_int(va_arg(ap,
                int), 16, 1);
                break;
            case 'p':
                pr_msg_ptr(va_arg(ap, uint64));
                break;
            case 's':
                if ((s = va_arg(ap, char*)) == 0)
                s = "(null)";
                for (; *s; s++)
                    putb(*s);
                break;
            case '%':
                putb('%');
                break;
            default:
                // Print unknown % sequence to draw attention.
                putb('%');
                putb(c);
                break;
        }
    }
    va_end(ap);

    putb('\n');
    release(&dmsg_buff.lock);

    return 0;
}


int sys_dmesg(void) {
    uint64 user_buff_ptr;
    int user_buffer_size;
    argaddr(0, &user_buff_ptr);
    argint(1, &user_buffer_size);

    acquire(&dmsg_buff.lock);
    if (user_buff_ptr == 0) {
        release(&dmsg_buff.lock);
        return -1;
    }

    // Уж лучше я скажу пользователю, что вероятно запись журнала не влезет в его буфер, чем давать ему обрубок журнала
    // (если в ситуацию с размером попадем, в любом случае copyout заноет, но хотя бы с этим чеком легче будет понять ошибку)
    if (user_buffer_size <= DIAG_MSG_BUFF_SIZE) {
        release(&dmsg_buff.lock);
        return -2;
    }

    if (dmsg_buff.tail < DIAG_MSG_BUFF_SIZE) {
        if (copyout(myproc()->pagetable, user_buff_ptr, &dmsg_buff.buff[dmsg_buff.head],
                    dmsg_buff.tail - dmsg_buff.head) < 0) {
            release(&dmsg_buff.lock);
            return -3;
        }
        release(&dmsg_buff.lock);
        return 0;
    }

    // Если все таки зациклилась очередь, соберем ее по кускам
    if (copyout(myproc()->pagetable, user_buff_ptr, &dmsg_buff.buff[dmsg_buff.head],
                DIAG_MSG_BUFF_SIZE - dmsg_buff.head) < 0) {
        release(&dmsg_buff.lock);
        return -4;
    }

    if (copyout(myproc()->pagetable, user_buff_ptr + (DIAG_MSG_BUFF_SIZE - dmsg_buff.head), &dmsg_buff.buff[0],
                dmsg_buff.tail - DIAG_MSG_BUFF_SIZE) < 0) {
        release(&dmsg_buff.lock);
        return -5;
    }

    release(&dmsg_buff.lock);
    return 0;
}

int sys_toggle_class_log(void) {
    int event_class, on_off;

    argint(0, &event_class);
    argint(1, &on_off);

    if (event_class >= NEVENTCLASSES || event_class < 0) return -1;
    if (on_off > 1 || on_off < 0) return -2;

    acquire(&ev_classes_logger.lock);
    ev_classes_logger.enabled_classes[event_class] = on_off;
    release(&ev_classes_logger.lock);
    return 0;
}

int sys_set_stop_ticks(void) {
    int query_ticks;
    argint(0, &query_ticks);

    int curr_ticks = ticks;
    if (query_ticks < curr_ticks) return -1;

    acquire(&ev_classes_logger.lock);
    ev_classes_logger.stop_ticks = curr_ticks + query_ticks;
    release(&ev_classes_logger.lock);

    return 0;
}