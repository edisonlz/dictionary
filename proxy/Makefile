CC = c99
CFLAGS = -Wall -O2

dict: clean main.c main.h
	$(CC) $(CFLAGS) -o dict main.c -D_GNU_SOURCE

clean:
	rm dict a.out -f


