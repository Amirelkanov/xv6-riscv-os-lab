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

int sys_pgaccess(uint64 addr, int len) {
  argaddr(0, &addr);
  argint(1, &len);

  pagetable_t pagetable = myproc()->pagetable;
  int is_accessed = 0;

  if (len < 0 || addr + len >= MAXVA) return -1;

  for (int va = PGROUNDDOWN(addr); va < addr + len; va += PGSIZE) {
    pte_t *pte = walk(pagetable, va, 0);
    if (pte == 0) return -1;
    if (*pte & PTE_A) {
      is_accessed = 1;
      *pte &= ~PTE_A; // clear PTE_A bits
    }
  }

  return is_accessed;
}