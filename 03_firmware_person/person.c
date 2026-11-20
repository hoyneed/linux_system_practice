#define _DEFAULT_SOURCE
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define WDT_SECONDS 10

static pid_t g_target_pid = 0;

static void wdt_handler(int sig) {
  (void)sig;
  if (g_target_pid > 0) {
    kill(g_target_pid, SIGTERM);
  }
  const char msg[] = "\n[WDT] Timeout! Firmware and Person shutting down...\n";
  ssize_t unused = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
  (void)unused;
  _exit(1);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <firmware_pid>\n", argv[0]);
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
  g_target_pid = (pid_t)val;

  if (kill(g_target_pid, 0) == -1) {
    perror("Target PID check failed");
    return 1;
  }

  setbuf(stdout, NULL);

  signal(SIGALRM, wdt_handler);
  alarm(WDT_SECONDS);

  printf("Person App started (target PID: %d, WDT: %ds)\n", (int)g_target_pid,
         WDT_SECONDS);
  printf("Commands: 1 (SIGUSR1), 2 (SIGUSR2), 3 (SIGTERM), exit\n");

  char buf[128];
  while (1) {
    printf("INPUT>> ");
    fflush(stdout);

    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      printf("\n");
      break;
    }

    if (strchr(buf, '\n') == NULL && !feof(stdin)) {
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF) {
      }
    }

    buf[strcspn(buf, "\r\n")] = '\0';

    /* Refresh Watchdog Timer on every input */
    alarm(WDT_SECONDS);

    if (buf[0] == '\0') {
      continue;
    }

    if (strcmp(buf, "1") == 0) {
      if (kill(g_target_pid, SIGUSR1) == -1) {
        perror("kill(SIGUSR1)");
      } else {
        printf("Sent SIGUSR1 to %d\n", (int)g_target_pid);
      }
    } else if (strcmp(buf, "2") == 0) {
      if (kill(g_target_pid, SIGUSR2) == -1) {
        perror("kill(SIGUSR2)");
      } else {
        printf("Sent SIGUSR2 to %d\n", (int)g_target_pid);
      }
    } else if (strcmp(buf, "3") == 0) {
      if (kill(g_target_pid, SIGTERM) == -1) {
        perror("kill(SIGTERM)");
      } else {
        printf("Sent SIGTERM to %d\n", (int)g_target_pid);
      }
    } else if (strcmp(buf, "exit") == 0) {
      alarm(0);
      printf("Person App exiting.\n");
      break;
    } else {
      printf("Invalid input '%s'. Please enter 1, 2, 3, or exit.\n", buf);
    }
  }

  alarm(0);
  return 0;
}
