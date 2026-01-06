CC = gcc
options = -g #-fsanitize=address

all: msgstat

msgstat: msgstat.c
	${CC} -Wall ${options} -o $@ -lgettextpo msgstat.c

leak:
	gcc -g -fsanitize=address -o test_leak test_leak.c -lgettextpo
	ASAN_OPTIONS=detect_leaks=1:fast_unwind_on_malloc=0 ./test_leak test.po
