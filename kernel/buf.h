struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint dev;
  uint blockno;
  struct sleeplock lock;
  uint refcnt;
  struct buf *prev; // LRU cache list
  struct buf *next;
  struct buf* out;
  uchar data[BSIZE];
};

#define NBUCKET 17
#define hash(blockno) ((blockno) % NBUCKET)
