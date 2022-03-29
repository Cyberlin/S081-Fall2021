// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock locks[NBUCKET];
  struct buf buf[NBUF];
  struct buf head[NBUCKET];
} bcache;
void
put(uint key, struct buf* val){
  struct buf* bkt = &bcache.head[hash(key)];
  val->next = bkt->next;
  bkt->next = val;
}
void
remove(struct buf* b){
  struct buf* e, *pre;
  e = bcache.head[hash(b->blockno)].next;
  pre = bcache.head[hash(b->blockno)].next;
  for(; e != 0; e=e->next){
    if(e == b){
      if(pre == e) {
        bcache.head[hash(b->blockno)].next = e->next;
      }else{
        pre->next = e->next;
      }
      e->next =0;
      return ;
    }
    if(pre != e){
      pre = pre->next;
    }
  }
}
void
debug_bucket(uint blockno){
  struct buf* e = 0;
  struct buf bkt = bcache.head[hash(blockno)];
  int cnt  = 0;
  for(e = bkt.next; e!=0; e=e->next){
    cnt ++;
    printf("blockno: %d  refcnt: %d\n",e->blockno, e->refcnt);
  }
  printf("-----------------------------------------^^^^cnt: %d^^^^\n",cnt);
}
void
binit(void)
{
  struct buf *b;

  for(int i = 0; i < NBUCKET; ++i) {
    initlock(&bcache.locks[i], "bcache.bucket");
    bcache.head[i].next = 0;
    // bcache.head[i].prev = &bcache.head[i];
    // bcache.head[i].next = &bcache.head[i];
  }
  //uint cnt = 0;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    // b->next = bcache.head[0].next;
    // b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    //cnt++;
    b->next = 0;
    put(0, b);
    
    // bcache.head[0].next->prev = b;
    // bcache.head[0].next = b;
  }
  //debug_bucket(0);
  
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b, *head, *freehead;

  acquire(&bcache.locks[hash(blockno)]);
  head = &bcache.head[hash(blockno)];

  // Is the block already cached?
  for(b = head->next; b != 0; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.locks[hash(blockno)]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(int i = 1; i < NBUCKET; ++i) {
    acquire(&bcache.locks[hash(blockno+i)]);
    //debug_bucket(blockno+i);
    freehead = &bcache.head[hash(blockno+i)];
    for(b = freehead->next; b != 0; b = b->next) {
      if(b->refcnt != 0) continue;
      remove(b);
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      // remove from free list
      // insert to head
      put(blockno, b);
      release(&bcache.locks[hash(blockno+i)]);
      release(&bcache.locks[hash(blockno)]);
      acquiresleep(&b->lock);
      return b;
    }
    release(&bcache.locks[hash(blockno+i)]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.locks[hash(b->blockno)]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    // b->next->prev = b->prev;
    // b->prev->next = b->next;
    // b->next = &bcache.head[hash(b->blockno)];
    // b->prev = bcache.head[hash(b->blockno)].prev;
    // bcache.head[hash(b->blockno)].prev->next = b;
    // bcache.head[hash(b->blockno)].prev = b;
  }
  
  release(&bcache.locks[hash(b->blockno)]);
}

void
bpin(struct buf *b) {
  acquire(&bcache.locks[hash(b->blockno)]);
  b->refcnt++;
  release(&bcache.locks[hash(b->blockno)]);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.locks[hash(b->blockno)]);
  b->refcnt--;
  release(&bcache.locks[hash(b->blockno)]);
}


