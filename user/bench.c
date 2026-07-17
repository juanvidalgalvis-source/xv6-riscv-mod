#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define CHILDREN 6
#define COUNT 30000000

// simple print lock pipe (created before fork so fd's are inherited)
int print_lock[2];

void child_work(void) {
  int pid = getpid();
  volatile long long x = 0;
  int lastp = -1;
  char token = 'x';

  for (long long i = 0; i < COUNT; i++) {
    x += i;
    if (i % (COUNT/20) == 0) { // report every 5%
      int pct = (int)(i * 100 / COUNT);
      if (pct != lastp) {
        lastp = pct;
        int t = uptime();
        // acquire lock
        if (read(print_lock[0], &token, 1) == 1) {
          printf("[hijo pid=%d] progreso=%d%% tick=%d\n", pid, pct, t);
          // release lock
          write(print_lock[1], &token, 1);
        }
      }
    }
  }

  // Try to allocate pages until sbrk fails
  int pages = 0;
  while (1) {
    char *r = sbrk(4096);
    if (r == SBRK_ERROR) break;
    pages++;
  }
  if (read(print_lock[0], &token, 1) == 1) {
    printf("[hijo pid=%d] logro asignar %d paginas antes de fallar sbrk()\n", pid, pages);
    write(print_lock[1], &token, 1);
  }
  exit(0);
}

int main(void) {
  int i;
  int pids[CHILDREN];
  int t0 = uptime();

  // create print lock and seed it with one token
  if (pipe(print_lock) < 0) {
    printf("pipe failed\n");
    exit(1);
  }
  char token = 'x';
  write(print_lock[1], &token, 1);

  for (i = 0; i < CHILDREN; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      child_work();
      // never returns
    }
    pids[i] = pid;
  }

  // print child pids so pids[] is used
  printf("padre: child pids =");
  for (i = 0; i < CHILDREN; i++) {
    printf(" %d", pids[i]);
  }
  printf("\n");

  for (i = 0; i < CHILDREN; i++) {
    wait(0);
  }
  int t1 = uptime();
  printf("padre: ticks transcurridos = %d\n", t1 - t0);
  exit(0);
}
