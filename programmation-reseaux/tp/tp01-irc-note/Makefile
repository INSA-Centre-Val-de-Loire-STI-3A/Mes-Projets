CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude

COMMON = $(wildcard src/common/*.c)
SERVER = $(wildcard src/server/*.c)
CLIENT = $(wildcard src/client/*.c)

all: bin/tcpserver bin/tcpclient bin/irc_server bin/irc_client

# partie 1 : echo TCP (autonome, comme au TD2)
bin/tcp%: src/echo/tcp%.c | bin
	$(CC) $(CFLAGS) -o $@ $<

bin/irc_server: $(SERVER) $(COMMON) | bin
	$(CC) $(CFLAGS) -o $@ $^

bin/irc_client: $(CLIENT) $(COMMON) | bin
	$(CC) $(CFLAGS) -o $@ $^

bin:
	mkdir -p bin

test: all
	./tests/test_echo.sh
	./tests/test_irc.sh

clean:
	rm -rf bin

.PHONY: all test clean
