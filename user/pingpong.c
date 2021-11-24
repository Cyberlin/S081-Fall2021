#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#define MAX_BUFFER 1024

int main(int argc, char *argv[]) {
  // create a pari of pipes
  int to_child[2];
  int to_parent[2];
  pipe(to_child);
  pipe(to_parent);
  // use fork to create child
  int pid;
  char msg[256];
  while ((pid = fork()) < 0)
    ;
  if (pid == 0) {
    // the child,read from the pipe and return it
    close(to_child[1]);
    close(to_parent[0]);

    read(to_child[0], &msg, MAX_BUFFER);
    printf("%d: received %s\n", getpid(), msg);
    close(to_child[0]);
    write(to_parent[1], "pong", 4);
    close(to_parent[1]);

    exit(0);
  } else {
    close(to_child[0]);
    close(to_parent[1]);
    write(to_child[1], "ping", 4);
    close(to_child[1]);

    read(to_parent[0], msg, MAX_BUFFER);
    printf("%d: received %s\n", getpid(), msg);
    close(to_parent[0]);
  }
  exit(0);
}
