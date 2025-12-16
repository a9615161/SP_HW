CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGETS = sig_speed_sigsuspend sig_speed_sigwaitinfo sysv_to_posix_signals

all: $(TARGETS)

sig_speed_sigsuspend: sig_speed_sigsuspend.c
	$(CC) $(CFLAGS) -o $@ $<

sig_speed_sigwaitinfo: sig_speed_sigwaitinfo.c
	$(CC) $(CFLAGS) -o $@ $<
sysv_to_posix_signals: sysv_to_posix_signals.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

test: all
	@echo "--- Testing sig_speed_sigsuspend (1,000,000 signals) ---"
	@time ./sig_speed_sigsuspend 1000000
	@echo "\n--- Testing sig_speed_sigwaitinfo (1,000,000 signals) ---"
	@time ./sig_speed_sigwaitinfo 1000000