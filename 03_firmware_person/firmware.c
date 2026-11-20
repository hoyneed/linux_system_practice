#define _DEFAULT_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t g_usr1_req = 0;
static volatile sig_atomic_t g_usr2_req = 0;
static volatile sig_atomic_t g_term_req = 0;

static void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        g_usr1_req = 1;
    } else if (sig == SIGUSR2) {
        g_usr2_req = 1;
    } else if (sig == SIGTERM) {
        g_term_req = 1;
    }
}

static void advance_char(char *ch, int *dir) {
    if (*dir == 1) {
        if (*ch < 'Z') {
            (*ch)++;
        } else {
            *dir = -1;
        }
    } else {
        if (*ch > 'A') {
            (*ch)--;
        } else {
            *dir = 1;
        }
    }
}

int main(void) {
    char ch = 'A';
    int dir = 1;

    setbuf(stdout, NULL);

    signal(SIGUSR1, handle_signal);
    signal(SIGUSR2, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("Firmware started (PID: %d)\n", (int)getpid());

    while (!g_term_req) {
        if (usleep(100000) == 0) {
            printf("%c ", ch);
            advance_char(&ch, &dir);
        }

        if (g_usr1_req) {
            g_usr1_req = 0;
            printf("\n[Button Clicked]\n");
        }

        if (g_usr2_req) {
            g_usr2_req = 0;
            printf("\033[2J\033[HRESET\n");
            ch = 'A';
            dir = 1;
        }
    }

    printf("\nBYE\n");
    return 0;
}
