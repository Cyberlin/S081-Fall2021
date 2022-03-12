// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#define PG_NUM  (((PHYSTOP) -(KERNBASE)) /PGSIZE)
void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;
static char coremap[PG_NUM];
char refcnt(uint64 pa){
  uint64 index = (pa -KERNBASE) >> 12;
  return coremap[index];
}
char inc_refcnt(uint64 pa){
  uint64 index = (pa -KERNBASE) >> 12;
  return ++coremap[index];
}
char dec_refcnt(uint64 pa){
  uint64 index = (pa -KERNBASE) >> 12;
  return --coremap[index];
}
void set_refcnt(uint64 pa){
  uint64 index = (pa -KERNBASE) >> 12;
  coremap[index] = 1;
}
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  memset(coremap, 0, PG_NUM);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  if(refcnt((uint64)pa) != 0){
    if(dec_refcnt((uint64)pa) > 0){
      return ;
    }
  }
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    inc_refcnt((uint64)r);
    memset((char*)r, 5, PGSIZE); // fill with junk
  }
  return (void*)r;
}
