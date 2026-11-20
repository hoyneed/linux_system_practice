#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * [실습용 스켈레톤] Day2 p.116 채굴기 프로젝트 (모의 실기 2)
 * 빌드: gcc -Wall -Wextra -o practice_miner practice_miner.c -lpthread
 */

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

/*
 * TODO 1: 채굴기 스레드 함수 구현
 * - void *miner_worker(void *arg)
 * - usleep(100000); (0.1초) 후 mutex를 잠그고 g_coins++ 수행 후 unlock
 * - 주의: mutex를 잠근 상태에서 usleep()을 호출하면 다른 스레드가 멈춥니다!
 */

/*
 * TODO 2: FIFO Queue 기반 add_miner() / del_miner() 구현
 * - add_miner: Miner 동적 할당 -> running = 1 -> pthread_create -> 큐 rear에 삽입
 * - del_miner: 큐 front에서 가장 오래된 Miner 꺼내기 -> running = 0 -> pthread_join -> free
 */

int main(void) {
    setbuf(stdout, NULL);

    /* TODO 3: 프로그램 시작 시 채굴기 1대 자동 생성 */
    printf("[PRACTICE] Queue capacity=%d, front=%d, rear=%d, count=%d, coins=%ld\n",
           MAX_MINERS, g_front, g_rear, g_count, g_coins);
    (void)g_queue;
    (void)g_coin_mutex;

    /*
     * TODO 4: "ssafy>> " 프롬프트 REPL 루프 구현
     * - fgets + 개행 제거 + strcmp로 "show", "add", "del", "exit" 분기
     * - show: g_coin_mutex 잠그고 g_coins 읽어서 출력
     * - 잘못된 명령어 처리 및 종료 시 모든 스레드/뮤텍스 정리
     */

    printf("Complete TODO 1~4 in practice_miner.c!\n");
    return 0;
}
