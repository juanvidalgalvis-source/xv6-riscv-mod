#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define CHILDREN 3
#define COUNT 200000000

void child_work(void) {
  int pid = getpid();
  volatile long long x = 0;
  int lastp = -1;
  for (long long i = 0; i < COUNT; i++) {
    x += i;
    if (i % (COUNT/20) == 0) { // report every 5%
      int pct = (int)(i * 100 / COUNT);
      int t = uptime();
      printf("[hijo pid=%d] progreso=%d%% tick=%d\n", pid, pct, t);
    }
  }

  // Try to allocate pages until sbrk fails
  int pages = 0;
  while (1) {
    char *r = sbrk(4096);
    if (r == SBRK_ERROR) break;
    pages++;
  }
  printf("[hijo pid=%d] logro asignar %d paginas antes de fallar sbrk()\n", pid, pages);
  exit(0);
}

int main(void) {
  int i;
  int pids[CHILDREN];
  int t0 = uptime();

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

  for (i = 0; i < CHILDREN; i++) {
    wait(0);
  }
  int t1 = uptime();
  printf("padre: ticks transcurridos = %d\n", t1 - t0);
  exit(0);
}
