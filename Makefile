all:
	$(CC) -D_DEFAULT_SOURCE -std=c99 -pedantic -o untracked untracked.c
