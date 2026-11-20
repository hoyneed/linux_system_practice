#define _DEFAULT_SOURCE
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/*
 * [실습용 스켈레톤] 시그널 기반 채굴기 제어 셸 (모의 실기 1)
 * 빌드: gcc -Wall -Wextra -o practice_ctl practice_ctl.c
 * 실행: ./miner_daemon & 로 데몬 실행 후 PID를 확인하고 ./practice_ctl <PID>
 */

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    /*
     * TODO 1: 실행 인자(argc) 개수 확인
     * - 인자가 2개(프로그램명 + PID)가 아니면 stderr에 Usage 출력 후 return 1;
     */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PID>\n", argv[0]);
        return 1;
    }

    /*
     * TODO 2: argv[1]을 strtol로 변환하고 pid_t 검증
     * - atoi("123abc")는 123을 반환하고 atoi("abc")는 0을 반환하므로
     *   strtol()로 변환 오류(errno, endptr == argv[1], *endptr != '\0')와
     *   범위(val <= 0 || val > INT_MAX)를 함께 검증하는 것이 안전합니다.
     * - kill(target_pid, 0) == -1 로 대상 프로세스 존재 여부를 확인하세요.
     */
    char *endptr = NULL;
    errno = 0;
    long val = strtol(argv[1], &endptr, 10);
    if (errno != 0 || endptr == argv[1] || *endptr != '\0' || val <= 0 || val > INT_MAX) {
        fprintf(stderr, "Invalid PID: %s\n", argv[1]);
        return 1;
    }
    pid_t target_pid = (pid_t)val;

    printf("[PRACTICE] Ready for PID %d. Complete TODO 3~6 in practice_ctl.c!\n", (int)target_pid);

    /*
     * TODO 3: 명령 입력 반복 루프(while (1)) 작성
     * - "ssafy>> " 프롬프트 출력 후 fflush(stdout);
     * - fgets(buf, sizeof(buf), stdin) == NULL 이면 EOF(Ctrl+D)이므로 break;
     * - buf[strcspn(buf, "\r\n")] = '\0'; 로 개행 문자 제거
     * - 빈 문자열(buf[0] == '\0')이면 continue;
     *
     * TODO 4: strcmp로 명령어 구분
     * - "show"        -> kill(target_pid, SIGINT)
     * - "add"         -> kill(target_pid, SIGUSR1)
     * - "del"         -> kill(target_pid, SIGUSR2)
     * - "stop"        -> kill(target_pid, SIGTERM)
     * - "kill"        -> kill(target_pid, SIGKILL)
     * - "exit"/"quit" -> break;
     *
     * TODO 5: kill() 반환값 확인 및 오류 처리
     * - kill() 반환값이 -1이면 perror("kill") 출력
     *
     * TODO 6: 잘못된 명령어 입력 시 안내 메시지 출력
     */

    return 0;
}
