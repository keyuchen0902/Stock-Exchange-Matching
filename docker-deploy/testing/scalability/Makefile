CC=g++
CFLAGs=-03
EXTRAFLAGS=-pthread

.Phony: all clean

all: client

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp $(EXTRAFLAGS)

clean:
	rm -f client *~