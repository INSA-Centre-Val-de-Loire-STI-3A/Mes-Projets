CC      = gcc
CFLAGS  = -Wall -Wextra -g

SRC = $(wildcard src/*.c)
BIN = $(patsubst src/%.c,bin/%,$(SRC))

all: $(BIN)

bin/%: src/%.c | bin
	$(CC) $(CFLAGS) -o $@ $<

bin:
	mkdir -p bin

clean:
	rm -rf bin

.PHONY: all clean
