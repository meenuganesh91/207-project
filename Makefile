##****************************************************************************
## Mind Sync Game Makefile
##
##****************************************************************************

CC=g++
CCFLAGS=-g -std=c++11 -D_POSIX_SOURCE -Wall

TARGETS=mindSync dataBase

all: $(TARGETS)

mindSync: mind_sync.o mind_sync_server.o server_common.o
	$(CC) $(CCFLAGS) -pthread -o $@ $^

mind_sync.o: mind_sync.cpp
	$(CC) $(CCFLAGS) -c $<

mind_sync_server.o: mind_sync_server.cpp server_common.o
	$(CC) $(CCFLAGS) -c $<

server_common.o: server_common.cpp
	$(CC) $(CCFLAGS) -c $<

dataBase: database_cnxn.o
	$(CC) $(CCFLAGS) -pthread -o $@ $^ -lsqlite3

database_cnxn.o: database_cnxn.cpp
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f *.o $(TARGETS)

