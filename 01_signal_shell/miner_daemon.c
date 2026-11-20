#define _DEFAULT_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t g_add_count = 0;
static volatile sig_atomic_t g_del_count = 0;
static volatile sig_atomic_t g_show_req = 0;
static volatile sig_atomic_t g_term_req = 0;

/* Async-signal-safe handler: only updates sig_atomic_t flags */
static void handle_signal(int sig) {
  if (sig == SIGUSR1) {
    g_add_count = g_add_count + 1;
  } else if (sig == SIGUSR2) {
    g_del_count = g_del_count + 1;
  } else if (sig == SIGINT) {
    g_show_req = 1;
  } else if (sig == SIGTERM) {
    g_term_req = 1;
  }
}

static void process_pending_events(long coins, int *miners) {
  sigset_t block_set, old_set;
  sigemptyset(&block_set);
  sigaddset(&block_set, SIGUSR1);
  sigaddset(&block_set, SIGUSR2);
  sigaddset(&block_set, SIGINT);

  /* Briefly block handled signals while consuming counters/flags */
  sigprocmask(SIG_BLOCK, &block_set, &old_set);
  sig_atomic_t add_n = g_add_count;
  sig_atomic_t del_n = g_del_count;
  sig_atomic_t show = g_show_req;
  g_add_count = 0;
  g_del_count = 0;
  g_show_req = 0;
  sigprocmask(SIG_SETMASK, &old_set, NULL);

  while (add_n > 0) {
    (*miners)++;
    printf("[DAEMON] SIGUSR1 received -> Miner added (miners: %d)\n", *miners);
    add_n--;
  }

  while (del_n > 0) {
    if (*miners > 0) {
      (*miners)--;
      printf("[DAEMON] SIGUSR2 received -> Miner removed (miners: %d)\n",
             *miners);
    } else {
      printf("[DAEMON] SIGUSR2 received -> No active miners to remove (miners: "
             "0)\n");
    }
    del_n--;
  }

  if (show) {
    printf("[DAEMON] SIGINT received -> Coins: %ld, Active miners: %d\n", coins,
           *miners);
  }
}

int main(void) {
  long coins = 0;
  int miners = 1;

  setbuf(stdout, NULL);

  signal(SIGUSR1, handle_signal);
  signal(SIGUSR2, handle_signal);
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  printf("[DAEMON] Miner started (PID: %d, miners: %d)\n", (int)getpid(),
         miners);

  while (!g_term_req) {
    /* Add coins only when the full 0.1s sleep completes without EINTR */
    if (usleep(100000) == 0) {
      coins += miners;
    }
    process_pending_events(coins, &miners);
  }

  process_pending_events(coins, &miners);
  printf("[DAEMON] SIGTERM received -> Shutting down (Final coins: %ld)\n",
         coins);
  return 0;
}
