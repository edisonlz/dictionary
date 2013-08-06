CC = c99
CFLAGS = -Wall -O2

dict: clean main.c main.h
	$(CC) $(CFLAGS) -o echo main.c network.c process.c -D_GNU_SOURCE

clean:
	rm echo a.out -f


