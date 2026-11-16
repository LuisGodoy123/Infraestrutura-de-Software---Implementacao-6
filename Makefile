CC = gcc
CFLAGS = -Wall -Wextra -std=c99
OBJS = main.o banker.o input.o output.o

banker: $(OBJS)
	$(CC) $(CFLAGS) -o banker $(OBJS)

%.o: %.c banker.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) banker result.txt

.PHONY: clean
