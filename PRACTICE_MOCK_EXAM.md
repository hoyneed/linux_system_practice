# 실전 모의고사 & 자가점검 퀴즈

실제 시험장과 같은 조건에서 아래 2개의 실기 문제와 15문항 퀴즈를 풀어보세요.

---

## 모의 실기 1 (유형 A — 시그널 기반 채굴기 제어 셸, 권장 30분)

### 상황 설명

백그라운드로 실행 중인 채굴기 데몬([`01_signal_shell/miner_daemon.c`](file:///home/hoyneed/linux_system_practice/01_signal_shell/miner_daemon.c))에 시그널을 보내 채굴기를 제어하는 C 제어 셸 프로그램([`01_signal_shell/practice_ctl.c`](file:///home/hoyneed/linux_system_practice/01_signal_shell/practice_ctl.c))을 완성하세요.

### 사전 준비 (백그라운드 데몬 실행 및 PID 찾기)

```bash
make all
./01_signal_shell/miner_daemon &
pidof miner_daemon      # 또는 pgrep -x miner_daemon / ps -ef | grep '[m]iner_daemon'
```

### 요구사항 명세

1. **실행 인자 검증**:
   - 실행 형태: `./practice_ctl <PID>`
   - 인자 개수가 2개가 아니면 `stderr`에 사용법을 출력하고 종료 코드 `1`로 종료한다.
   - `argv[1]`을 `strtol()`로 변환하여 `errno`, `endptr`, `pid <= 0 || val > INT_MAX`를 검증하고 오류 시 종료 코드 `1`로 종료한다.
   - `kill(pid, 0) == -1`로 대상 프로세스 존재 여부를 사전 확인한다.
2. **대화형 셸 루프**:
   - 프롬프트 `"ssafy>> "`를 출력하고 즉시 화면에 보이도록 버퍼를 비운다(`fflush(stdout)`).
   - `fgets()`로 한 줄을 입력받고 `NULL`(EOF)이면 루프를 종료한다.
   - 개행 문자(`\n`, `\r\n`)를 제거하고, 빈 입력(`""`)이면 무시하고 다음 프롬프트를 출력한다.
3. **명령어 ↔ 시그널 매핑 (`strcmp`)**:
   - `show` → `SIGINT` 전송
   - `add` → `SIGUSR1` 전송
   - `del` → `SIGUSR2` 전송
   - `stop` → `SIGTERM` 전송
   - `kill` → `SIGKILL` 전송
   - `exit` 또는 `quit` → 제어 셸 정상 종료
   - 그 외 입력 → 잘못된 명령어 안내 메시지 출력 후 루프 계속
4. **오류 처리 및 빌드**:
   - 모든 `kill()` 호출의 반환값을 확인해 `-1`이면 `perror("kill")`을 출력한다.
   - `gcc -Wall -Wextra -o 01_signal_shell/practice_ctl 01_signal_shell/practice_ctl.c` 빌드 시 경고 0건이어야 한다.

### 채점 기준표 (100점)

| 항목 | 배점 | 확인 포인트 |
| --- | ---: | --- |
| 실행 인자 및 PID 검증 | 25 | `argc != 2`, `strtol` (`endptr`, `errno`), `pid <= 0`, `kill(pid, 0)` |
| 프롬프트 및 `fgets` 입력 처리 | 20 | `fflush(stdout)`, EOF 처리, 개행 제거, 빈 줄 무시 |
| 명령어 분기 및 `kill()` 전송 | 35 | `strcmp == 0`, 5종 시그널 정확히 매핑, `exit` |
| 예외/오류 처리 및 무경고 빌드 | 20 | 잘못된 명령 안내, `kill()` 반환값 체크, `-Wall -Wextra` 통과 |

- 참고 모범 답안: [`01_signal_shell/miner_ctl.c`](file:///home/hoyneed/linux_system_practice/01_signal_shell/miner_ctl.c)

---

## 모의 실기 2 (유형 B — Day2 p.116 멀티스레드 채굴기 셸, 권장 40분)

### 상황 설명

Day2 116페이지 `[도전 4] 채굴기 프로젝트` 요구사항을 만족하는 멀티스레드 채굴기 셸([`02_thread_miner/practice_miner.c`](file:///home/hoyneed/linux_system_practice/02_thread_miner/practice_miner.c))을 완성하세요.

### 요구사항 명세

1. **초기 실행**:
   - 프로그램 실행 시 채굴기(Thread) 1대가 자동으로 생성되어 채굴을 시작한다.
   - 각 채굴기는 **0.1초(`usleep(100000)`)마다 코인을 1씩 증가**시킨다.
   - 프롬프트 `"ssafy>> "`를 출력하고 명령을 대기한다.
2. **명령어 동작**:
   - `show`: 현재까지 채굴한 총 코인 개수를 출력한다.
   - `add`: 채굴기 스레드를 1대 추가한다.
   - `del`: **FIFO Queue 자료구조**를 이용해 가장 오래된 채굴기 1대를 제거한다 (채굴기가 0대일 때 예외 처리).
   - `exit`: 실행 중인 모든 채굴기 스레드와 `pthread_mutex_t`를 안전하게 정리한 뒤 종료한다.
   - 잘못된 입력: 오류 메시지를 출력하고 프롬프트를 다시 띄운다.
3. **동기화 & 빌드**:
   - 공유 코인 변수는 반드시 `pthread_mutex_lock` / `pthread_mutex_unlock`으로 보호한다.
   - `gcc -Wall -Wextra -o 02_thread_miner/practice_miner 02_thread_miner/practice_miner.c -lpthread` 빌드 시 경고가 없어야 한다.

### 채점 기준표 (100점)

| 항목 | 배점 | 확인 포인트 |
| --- | ---: | --- |
| 스레드 생성 및 0.1초 채굴 동작 | 25 | 시작 시 1대 자동 시작, `usleep(100000)` |
| `pthread_mutex` 동기화 | 25 | Critical Section 보호, sleep 중 락 점유 금지 |
| FIFO Queue 기반 `add` / `del` | 30 | 가장 오래된 채굴기 우선 제거, 스레드 자원 회수(`pthread_join`) |
| 셸 입력 처리 및 예외 처리 | 20 | `ssafy>>`, `show`, 빈 큐 `del`, 잘못된 명령, 무경고 빌드 |

- 참고 모범 답안: [`02_thread_miner/miner_thread.c`](file:///home/hoyneed/linux_system_practice/02_thread_miner/miner_thread.c)

---

## 모의 필기 / 단답 자가점검 퀴즈 (15문항)

**Q1.** `signal()`로 핸들러를 등록하지 않은 프로세스에 `SIGUSR1` 또는 `SIGUSR2`를 보내면 어떤 일이 벌어지나요?
<details>
<summary>정답 및 해설 보기</summary>
<strong>프로세스가 종료(Terminate)됩니다.</strong> <code>SIGUSR1</code>과 <code>SIGUSR2</code>의 기본 동작(Default Action)은 무시가 아니라 종료입니다.
</details>

**Q2.** 시그널 중 프로세스가 핸들러로 잡거나(catch) 무시(ignore)할 수 없는 시그널 2가지는 무엇인가요?
<details>
<summary>정답 및 해설 보기</summary>
<strong><code>SIGKILL(9)</code>과 <code>SIGSTOP(19)</code></strong>입니다.
</details>

**Q3.** `SIGTERM(15)`과 `SIGKILL(9)`의 가장 큰 차이점은 무엇인가요?
<details>
<summary>정답 및 해설 보기</summary>
<code>SIGTERM</code>은 핸들러를 등록해 열린 파일·메모리·스레드를 정리한 뒤 안전하게 종료(Graceful Shutdown)할 수 있지만, <code>SIGKILL</code>은 커널이 프로세스를 즉시 강제 종료하므로 정리 코드를 실행할 수 없습니다.
</details>

**Q4.** `ps -ef | grep miner` 실행 시 검색 결과에 `grep miner` 줄까지 같이 출력되는 것을 피하는 방법 2가지는 무엇인가요?
<details>
<summary>정답 및 해설 보기</summary>
1) <code>ps -ef | grep miner | grep -v grep</code><br/>
2) <code>ps -ef | grep '[m]iner'</code> (정규식 <code>[m]</code>은 <code>m</code>과 매치되지만 명령줄 문자열 <code>'[m]iner'</code> 자체는 매치되지 않음)
</details>

**Q5.** `pgrep`과 `pidof`의 주요 차이점 한 가지는 무엇인가요?
<details>
<summary>정답 및 해설 보기</summary>
<code>pgrep</code>은 기본적으로 프로세스 이름의 <strong>부분 문자열(정규식 패턴)</strong>로 검색하며(정확 일치는 <code>-x</code>), <code>pidof</code>는 <strong>정확한 프로그램 이름</strong>과 일치하는 프로세스의 PID를 찾습니다.
</details>

**Q6.** 터미널에서 `./miner_daemon`을 백그라운드로 실행하는 명령과, 특정 PID `1234`에 `SIGUSR1`을 보내는 셸 명령을 쓰세요.
<details>
<summary>정답 및 해설 보기</summary>
백그라운드 실행: <code>./miner_daemon &amp;</code><br/>
시그널 전송: <code>kill -SIGUSR1 1234</code> (또는 <code>kill -USR1 1234</code>, <code>kill -10 1234</code>)
</details>

**Q7.** `kill -0 1234` 명령(또는 C에서 `kill(1234, 0)`)은 무엇을 할 때 사용하나요?
<details>
<summary>정답 및 해설 보기</summary>
실제 시그널을 보내지 않고, <strong>PID 1234 프로세스가 현재 살아 있는지와 시그널 전송 권한이 있는지 확인</strong>할 때 사용합니다.
</details>

**Q8.** C 코드에서 `kill()` 시스템 콜을 쓰기 위해 필요한 헤더와 함수 원형, 반환값 규칙을 적으세요.
<details>
<summary>정답 및 해설 보기</summary>
헤더: <code>#include &lt;sys/types.h&gt;</code>, <code>#include &lt;signal.h&gt;</code><br/>
원형: <code>int kill(pid_t pid, int sig);</code><br/>
반환값: 성공 시 <code>0</code>, 실패 시 <code>-1</code> (<code>errno</code> 설정).
</details>

**Q9.** 사용자가 `./ctl abc`를 입력했을 때 `pid_t pid = atoi(argv[1]); kill(pid, SIGTERM);` 코드가 매우 위험한 이유는 무엇인가요?
<details>
<summary>정답 및 해설 보기</summary>
<code>atoi("abc")</code>는 실패 시 <code>0</code>을 반환합니다. <code>kill(0, SIGTERM)</code>은 <strong>현재 프로세스 그룹에 속한 모든 프로세스(자기 자신과 셸 포함)</strong>에 종료 시그널을 보내버리므로 반드시 <code>if (pid &lt;= 0)</code> 검사가 필요합니다.
</details>

**Q10.** `printf("ssafy>> ");` 직후 `fgets()`가 실행될 때 프롬프트가 화면에 보이지 않을 수 있는 이유와 해결책은?
<details>
<summary>정답 및 해설 보기</summary>
표준 출력(<code>stdout</code>)은 기본적으로 개행(<code>\n</code>)이 나올 때까지 버퍼링되기 때문입니다. <code>fflush(stdout);</code>을 호출하거나 프로그램 시작 시 <code>setbuf(stdout, NULL);</code>로 버퍼링을 끄면 해결됩니다.
</details>

**Q11.** `fgets(buf, sizeof(buf), stdin)`로 `"show"`를 입력받았을 때 `strcmp(buf, "show") == 0`이 거짓이 되는 이유와 1줄짜리 해결 코드는?
<details>
<summary>정답 및 해설 보기</summary>
<code>fgets()</code>는 입력 끝의 개행 문자 <code>'\n'</code>까지 버퍼에 포함하기 때문입니다(<code>"show\n"</code>).<br/>
해결 코드: <code>buf[strcspn(buf, "\r\n")] = '\0';</code>
</details>

**Q12.** 사용자가 터미널에서 `Ctrl + D`(또는 파이프 끝)를 입력하면 `fgets()`는 무엇을 반환하며, 이를 처리하지 않으면 어떤 현상이 생기나요?
<details>
<summary>정답 및 해설 보기</summary>
<code>NULL</code>을 반환합니다. 반환값을 체크하지 않으면 <code>buf</code>에 이전 값이 남거나 쓰레기 값을 계속 비교하며 <strong>무한 루프</strong>에 빠집니다.
</details>

**Q13.** `./my_shell 4512`로 실행했을 때 `main(int argc, char *argv[])`에서 `argc`, `argv[0]`, `argv[1]`의 값은 각각 무엇인가요?
<details>
<summary>정답 및 해설 보기</summary>
<code>argc == 2</code>, <code>argv[0] == "./my_shell"</code>, <code>argv[1] == "4512"</code>
</details>

**Q14.** `gcc -o sample.c sample`처럼 `-o` 뒤의 순서를 반대로 쓰면 어떤 문제가 발생하나요?
<details>
<summary>정답 및 해설 보기</summary>
<code>-o</code> 바로 뒤의 인자가 출력 파일명이므로 <code>sample.c</code>가 출력 파일로 지정되어 기존 소스 코드가 손실되거나, 존재하지 않는 입력 파일 <code>sample</code> 때문에 컴파일이 실패합니다. 올바른 순서는 <code>gcc -o sample sample.c</code>입니다.
</details>

**Q15.** 멀티스레드 채굴기에서 `pthread_mutex_lock(&m)` 안에 `usleep(100000)`을 넣으면 안 되는 이유는 무엇인가요?
<details>
<summary>정답 및 해설 보기</summary>
한 스레드가 뮤텍스를 잠근 상태로 0.1초 동안 잠들면 다른 모든 채굴기 스레드와 <code>show</code> 명령까지 뮤텍스 대기열에 막혀 <strong>사실상 단일 스레드처럼 직렬화되거나(병목) <code>pthread_cancel</code> 시 데드락이 발생</strong>할 수 있습니다. <code>usleep()</code>은 반드시 락 바깥에서 호출하고, 공유 변수 <code>coins++</code> 구간만 최소한으로 잠가야 합니다.
</details>
