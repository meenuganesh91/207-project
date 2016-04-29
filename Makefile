##****************************************************************************
## Mind Sync Game Makefile
##
##****************************************************************************

CC=g++
CCFLAGS=-g -std=c++11 -D_POSIX_SOURCE -Wall

TARGETS=mindSync

all: $(TARGETS)

mindSync: mind_sync.o server_common.o
	$(CC) $(CCFLAGS) -pthread -o $@ $^

mind_sync.o: mind_sync.cpp
	$(CC) $(CCFLAGS) -c $<

connect_server.o: server_common.cpp
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f *.o $(TARGETS)

