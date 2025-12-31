CC = gcc
options = -g #-fsanitize=address

all: msgstat

msgstat: msgstat.c
	${CC} -Wall ${options} -o $@ -lgettextpo msgstat.c
