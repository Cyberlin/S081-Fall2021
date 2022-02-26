#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "sysinfo.h"

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
uint64
sys_trace(void){
  int syscall_num;
  int res;

  if(argint(0,&syscall_num)<0){
    return -1;
  }
  enable_trace(syscall_num);
  struct proc* process = myproc();
  int pid = process->pid;
  int trace_num = process->trace_num;
  res=0;
  if(trace_num&(1<<SYS_trace)){
    printf("%d: syscall trace -> %d\n",pid,res);
  }
  return res;
}
uint64
sys_sysinfo(void){
  uint64 si;//user pointer to struct sysinfo
  struct sysinfo sysinfo;
  struct proc* p = myproc();

  if(argaddr(0,&si)<0){
    return -1;
  }
  //update the sysinfo
  int nproc = get_nproc();
  int freemem = get_freemem();
  sysinfo.nproc = nproc;
  sysinfo.freemem =freemem;
  if(copyout(p->pagetable,si,(char*)&sysinfo,sizeof(sysinfo))<0){
    return -1;
  }
  return 0;
}

