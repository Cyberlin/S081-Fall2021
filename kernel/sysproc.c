#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


//#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.

  //get the args;
  
  uint64 buf, abits;
  uint64 bitmask = 0;
  int page_nums;
  if(argaddr(0, &buf) || argint(1, &page_nums) || argaddr(2, &abits) < 0 || page_nums > 64){
    return -1;
  }
  //printf("buf=%p, page_nums=%d, abits=%p\n",buf, page_nums, abits);

  struct proc * p = myproc();
  //limits the max size
  uint64 va = buf;
  pte_t *pte;
  int a_set;

  for(uint64 i = 0; i < page_nums; i++){
    va = buf + i * PGSIZE;
    pte = walk(p->pagetable, va, 1);

    if(pte == 0){
      return -1;
    }
    a_set = (*pte) & PTE_A;
    if(a_set){
      bitmask |= (1 << i);
      *pte &= ~(PTE_A);
      //printf("page: %d\n",i);
      //printf("*pte: %p, *pte_clear: %p\n",*pte, *pte & ~(PTE_A));
    }
  }
  //printf("bitmask: %p\n",bitmask);
  copyout(p->pagetable, abits, (char*)(&bitmask), sizeof(int));
  return 0;
}
//#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
