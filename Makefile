all:
	$(CC) -D_DEFAULT_SOURCE $(CCFLAGS) -std=c99 -pedantic -o untracked untracked.c
