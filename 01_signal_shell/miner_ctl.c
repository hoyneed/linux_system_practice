#define _DEFAULT_SOURCE
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static int send_signal(pid_t pid, int sig, const char *sig_name) {
  if (kill(pid, sig) == -1) {
    if (errno == ESRCH) {
      fprintf(stderr, "[CTL] Target process (PID: %d) does not exist.\n",
              (int)pid);
    }
    perror("kill");
    return -1;
  }
  printf("[CTL] Sent %s (%d) to PID %d\n", sig_name, sig, (int)pid);
  return 0;
}

static void print_help(void) {
  printf("Available commands:\n");
  printf("  show        -> send SIGINT  (show coins & active miners)\n");
  printf("  add         -> send SIGUSR1 (add 1 miner)\n");
  printf("  del         -> send SIGUSR2 (remove 1 miner)\n");
  printf("  stop        -> send SIGTERM (graceful shutdown)\n");
  printf("  kill        -> send SIGKILL (force kill)\n");
  printf("  help        -> show this help\n");
  printf("  exit | quit -> exit controller shell\n");
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <PID>\n", argv[0]);
    return 1;
  }

  char *endptr = NULL;
  errno = 0;
  long val = strtol(argv[1], &endptr, 10);
  if (errno != 0 || endptr == argv[1] || *endptr != '\0' || val <= 0 ||
      val > INT_MAX) {
    fprintf(stderr, "Invalid PID: %s\n", argv[1]);
    return 1;
  }
  pid_t target_pid = (pid_t)val;

  if (kill(target_pid, 0) == -1) {
    perror("Target PID check failed");
    return 1;
  }

  setbuf(stdout, NULL);
  printf("[CTL] Connected to target PID %d (type 'help' for commands)\n",
         (int)target_pid);

  char buf[256];
  while (1) {
    printf("ssafy>> ");
    fflush(stdout);

    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      printf("\n");
      break;
    }

    /* Discard trailing characters if the input line exceeded buf size */
    if (strchr(buf, '\n') == NULL && !feof(stdin)) {
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF) {
      }
    }

    buf[strcspn(buf, "\r\n")] = '\0';

    if (buf[0] == '\0') {
      continue;
    }

    if (strcmp(buf, "show") == 0) {
      send_signal(target_pid, SIGINT, "SIGINT");
    } else if (strcmp(buf, "add") == 0) {
      send_signal(target_pid, SIGUSR1, "SIGUSR1");
    } else if (strcmp(buf, "del") == 0) {
      send_signal(target_pid, SIGUSR2, "SIGUSR2");
    } else if (strcmp(buf, "stop") == 0) {
      send_signal(target_pid, SIGTERM, "SIGTERM");
    } else if (strcmp(buf, "kill") == 0) {
      send_signal(target_pid, SIGKILL, "SIGKILL");
    } else if (strcmp(buf, "help") == 0) {
      print_help();
    } else if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0) {
      printf("Bye!\n");
      break;
    } else {
      printf("Unknown command: '%s' (type 'help')\n", buf);
    }
  }

  return 0;
}
