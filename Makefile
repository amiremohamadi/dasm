CC ?= cc

dasm: *.h *.c
	$(CC) *.c -o dasm
