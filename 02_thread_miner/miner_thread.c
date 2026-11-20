#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_MINERS 64

typedef struct {
    pthread_t tid;
    int id;
    volatile int running;
} Miner;

static long g_coins = 0;
static pthread_mutex_t g_coin_mutex = PTHREAD_MUTEX_INITIALIZER;

static Miner *g_queue[MAX_MINERS];
static int g_front = 0;
static int g_rear = 0;
static int g_count = 0;
static int g_next_id = 1;

/*
 * Note on pthread_cancel vs running flag:
 * When stopping a thread with pthread_cancel() (Day2 p.109), cancelling a thread
 * while it holds g_coin_mutex would cause a deadlock. Always sleep outside the
 * mutex lock, or use a clean exit flag + pthread_join() as implemented here.
 */
static void *miner_worker(void *arg) {
    Miner *m = (Miner *)arg;
    while (m->running) {
        usleep(100000); /* 0.1 second */
        if (!m->running) {
            break;
        }
        pthread_mutex_lock(&g_coin_mutex);
        g_coins++;
        pthread_mutex_unlock(&g_coin_mutex);
    }
    return NULL;
}

static int add_miner(void) {
    if (g_count >= MAX_MINERS) {
        printf("[MINER] Queue full (max: %d)\n", MAX_MINERS);
        return -1;
    }

    Miner *m = (Miner *)malloc(sizeof(Miner));
    if (m == NULL) {
        perror("malloc");
        return -1;
    }

    m->id = g_next_id++;
    m->running = 1;

    if (pthread_create(&m->tid, NULL, miner_worker, m) != 0) {
        fprintf(stderr, "[MINER] pthread_create failed\n");
        free(m);
        return -1;
    }

    g_queue[g_rear] = m;
    g_rear = (g_rear + 1) % MAX_MINERS;
    g_count++;

    printf("[MINER] Miner #%d added (Active miners: %d)\n", m->id, g_count);
    return 0;
}

static int del_miner(void) {
    if (g_count == 0) {
        printf("[MINER] No active miners to remove.\n");
        return -1;
    }

    /* FIFO dequeue: remove the oldest miner */
    Miner *m = g_queue[g_front];
    g_queue[g_front] = NULL;
    g_front = (g_front + 1) % MAX_MINERS;
    g_count--;

    m->running = 0;
    pthread_join(m->tid, NULL);

    printf("[MINER] Oldest Miner #%d removed (Active miners: %d)\n", m->id, g_count);
    free(m);
    return 0;
}

static void show_status(void) {
    pthread_mutex_lock(&g_coin_mutex);
    long coins = g_coins;
    pthread_mutex_unlock(&g_coin_mutex);

    printf("[SHOW] Coins: %ld (Active miners: %d)\n", coins, g_count);
}

static void cleanup_all_miners(void) {
    while (g_count > 0) {
        del_miner();
    }
}

int main(void) {
    setbuf(stdout, NULL);

    /* Start with 1 miner automatically */
    if (add_miner() != 0) {
        return 1;
    }

    char buf[256];
    while (1) {
        printf("ssafy>> ");
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

        if (buf[0] == '\0') {
            continue;
        }

        if (strcmp(buf, "show") == 0) {
            show_status();
        } else if (strcmp(buf, "add") == 0) {
            add_miner();
        } else if (strcmp(buf, "del") == 0) {
            del_miner();
        } else if (strcmp(buf, "help") == 0) {
            printf("Commands: show, add, del, help, exit\n");
        } else if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0) {
            printf("Shutting down miners...\n");
            break;
        } else {
            printf("Unknown command: '%s' (type 'help')\n", buf);
        }
    }

    cleanup_all_miners();
    pthread_mutex_destroy(&g_coin_mutex);
    return 0;
}
