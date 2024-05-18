#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void) {
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void) {
  return myproc()->pid;
}

uint64
sys_fork(void) {
  return fork();
}

uint64
sys_wait(void) {
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void) {
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void) {
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void) {
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void) {
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_ps_listinfo(void) {
  procinfo_t *plist;
  int lim;

  argaddr(0, (uint64 *) &plist);
  argint(1, &lim);

  return ps_listinfo(plist, lim);
}

int sys_pgaccess(uint64 first_user_page_addr, int num_of_pages, uint64 res_buff_addr) {
  argaddr(0, &first_user_page_addr);
  argint(1, &num_of_pages);
  argaddr(2, &res_buff_addr);

  pagetable_t pagetable = myproc()->pagetable;

  char is_accessed[num_of_pages + 1];
  is_accessed[num_of_pages] = 0; // Мб эта строчка не нужна, т.к. вроде бы как char[] заполняется '\0', но лучше перестраховаться

  for (int i = 0; i < num_of_pages; i++) {
    pte_t *pte = walk(pagetable, first_user_page_addr + PGSIZE * i, 0);
    if (*pte & PTE_A) {
      is_accessed[i] = '1';
      *pte &= ~PTE_A; // clear PTE_A bits
    } else {
      is_accessed[i] = '0';
    }
  }

  if (copyout(pagetable, res_buff_addr, is_accessed, sizeof(is_accessed)) < 0) return 1;

  return 0;
}