#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define CHILDREN 3
#define COUNT 5000000

// We'll use a pipe per child to let the parent signal the child to start

// simple print lock pipe (created before fork so fd's are inherited)
int print_lock[2];

void child_work(int rd) {
  int pid = getpid();
  // wait for parent to write a byte on the pipe
  char c;
  if (read(rd, &c, 1) != 1) {
    // pipe read failed, but continue
  }
  // close read end
  close(rd);

  volatile long long x = 0;
  for (long long i = 0; i < COUNT; i++) {
    x += i;
    if (i % (COUNT/20) == 0) {
      int pct = (int)(i * 100 / COUNT);
      int t = uptime();
      printf("[hijo pid=%d] progreso=%d%% tick=%d\n", pid, pct, t);
    }
  }

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
  int pipes[CHILDREN][2];
  int t0 = uptime();

  // create print lock and seed it with one token
  if (pipe(print_lock) < 0) {
    printf("pipe failed\n");
    exit(1);
  }
  char token = 'x';
  write(print_lock[1], &token, 1);

  for (i = 0; i < CHILDREN; i++) {
    if (pipe(pipes[i]) < 0) {
      printf("pipe failed\n");
      exit(1);
    }
    int pid = fork();
    if (pid < 0) {
      printf("fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      // child
      close(pipes[i][1]); // close write end
      child_work(pipes[i][0]);
      // never returns
    }
    // parent
    pids[i] = pid;
    close(pipes[i][0]); // close read end in parent
  }

  // parent assigns priorities and memlimits, then signals children
  // priorities: 2,10,18 ; memlimits: 200,50,10
  set_priority(pids[0], 2);
  set_memlimit(pids[0], 200);
  write(pipes[0][1], "x", 1);
  close(pipes[0][1]);

  set_priority(pids[1], 10);
  set_memlimit(pids[1], 50);
  write(pipes[1][1], "x", 1);
  close(pipes[1][1]);

  set_priority(pids[2], 18);
  set_memlimit(pids[2], 10);
  write(pipes[2][1], "x", 1);
  close(pipes[2][1]);

  for (i = 0; i < CHILDREN; i++) {
    wait(0);
  }
  int t1 = uptime();
  printf("padre: ticks transcurridos = %d\n", t1 - t0);
  exit(0);
}
