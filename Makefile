CC = gcc
CFLAGS = -Wall -Wextra

SIGNAL_TARGETS = 01_signal_shell/miner_daemon 01_signal_shell/miner_ctl
THREAD_TARGETS = 02_thread_miner/miner_thread
FW_TARGETS = 03_firmware_person/firmware 03_firmware_person/person
PRACTICE_TARGETS = 01_signal_shell/practice_ctl 02_thread_miner/practice_miner

.PHONY: all practice test clean

all: $(SIGNAL_TARGETS) $(THREAD_TARGETS) $(FW_TARGETS)

practice: $(PRACTICE_TARGETS)

01_signal_shell/miner_daemon: 01_signal_shell/miner_daemon.c
	$(CC) $(CFLAGS) -o $@ $<

01_signal_shell/miner_ctl: 01_signal_shell/miner_ctl.c
	$(CC) $(CFLAGS) -o $@ $<

01_signal_shell/practice_ctl: 01_signal_shell/practice_ctl.c
	$(CC) $(CFLAGS) -o $@ $<

02_thread_miner/miner_thread: 02_thread_miner/miner_thread.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

02_thread_miner/practice_miner: 02_thread_miner/practice_miner.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

03_firmware_person/firmware: 03_firmware_person/firmware.c
	$(CC) $(CFLAGS) -o $@ $<

03_firmware_person/person: 03_firmware_person/person.c
	$(CC) $(CFLAGS) -o $@ $<

test: all practice
	./verify.sh

clean:
	rm -f $(SIGNAL_TARGETS) $(THREAD_TARGETS) $(FW_TARGETS) $(PRACTICE_TARGETS)
