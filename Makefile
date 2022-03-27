CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -pthread

all: server

server: tinyxml2.h server.cpp database.h account.h account.cpp position.h position.cpp transcation.h transcation.cpp functions.h functions.cpp execution.h execution.cpp 
	$(CC) $(CFLAGS) -o server tinyxml2.cpp server.cpp account.cpp position.cpp transcation.cpp functions.cpp execution.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o server

clobber:
	rm -f *~ *.o