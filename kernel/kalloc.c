// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

extern struct cpu cpus[NCPU];

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct run* freelist;

void
kinit()
{
  //initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}
int
getnpg(void* pa_start, void* pa_end)
{
    char* p;
    int npg = 0;
    p = (char*)PGROUNDUP((uint64)pa_start);
    for (; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
        npg++;
    }
    return npg;
}
void
pgfree(void *pa){
  struct run* r;

  if((uint64)pa % PGSIZE != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP){
    panic("pgfree");
  }
  
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  r->next = freelist;
  freelist = r;
}
void
pg_nextn(struct run** pp, uint64 num){
  while(num --){
    *pp = (*pp)->next;
  }
}
void
debug_ncfreelist(int hartid){
   struct run* temp = cpus[hartid].kmem.cfreelist;
     uint64 tn = 0;
     while(temp){
       temp = temp ->next;
       tn ++;
     }
      printf("cpus[%d]: tn =%d start=%p\n",hartid ,tn,cpus[hartid].kmem.cfreelist);
}
void
cpufrlst_split(uint64 npg){
  struct run* p = freelist;
  struct run* pre = 0;
  char lockname[10];
  for(int i = 0; i < NCPU; i++){
   if(i == 0){
     cpus[i].kmem.cfreelist = p;
     snprintf(lockname, 10, "kmemlock%d",i);
     initlock(&cpus[i].kmem.lock, lockname);
     //next cfreelist
     pg_nextn(&p, (npg % NCPU + npg / NCPU));

     pre = (struct run* )((uint64)p + PGSIZE);
     pre->next = 0;
     
     //debug_ncfreelist(i);
    
   }else{ 
     cpus[i].kmem.cfreelist = p;
     snprintf(lockname, 10, "kmemlock%d",i);
     initlock(&cpus[i].kmem.lock, lockname);

     pg_nextn(&p, npg / NCPU);
     
     pre = (struct run* )((uint64)p + PGSIZE);
     pre->next = 0;

     //debug_ncfreelist(i);

   }
  }
}
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  uint64 npg = 0;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    pgfree(p);
    npg++;
  }
  //printf("npg = %d\n",npg);
  cpufrlst_split(npg);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  //append the free page to own freelist
  struct run* r;
  
  if((uint64)pa % PGSIZE != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP){
    panic("kfree");
  }
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  push_off();
  r->next = cpus[cpuid()].kmem.cfreelist;
  cpus[cpuid()].kmem.cfreelist = r;
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run* r;
  uint64 curid;

  push_off();
  curid = cpuid();
  pop_off();

  //try to fetch a pgsize from own freelist
  struct run* curfreelist = cpus[curid].kmem.cfreelist;
  r = curfreelist;
  if(curfreelist == 0){
    //steal from others
    for(int i = 0; i < NCPU; i++){
      if(i == curid){
        continue;
      }
      //borrow from other NCPU
      if(cpus[i].kmem.cfreelist != 0){
        acquire(&cpus[i].kmem.lock);
        r = cpus[i].kmem.cfreelist;
        cpus[i].kmem.cfreelist = r->next;
        release(&cpus[i].kmem.lock);
        break;
      }
    }
  }else{
    cpus[curid].kmem.cfreelist = r->next;
  }

  if(r){
    memset((char*)r, 5, PGSIZE);
  }
  
  return (void*)r;
}
