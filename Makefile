CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -pthread

all: server

server: tinyxml2.h server.cpp database.h handledb.h handledb.cpp functions.h functions.cpp
	$(CC) $(CFLAGS) -o server tinyxml2.cpp server.cpp handledb.cpp functions.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o server

clobber:
	rm -f *~ *.o