
all: discover

discover: discover.c homeplug-av.o rawsocket.o
	gcc -g -Wall discover.c homeplug-av.o rawsocket.o -o discover

rawsocket.o: rawsocket.c rawsocket.h
	gcc -g -Wall -c rawsocket.c -o rawsocket.o

homeplug-av.o: homeplug-av.c homeplug-av.h
	gcc -g -Wall -c homeplug-av.c -o homeplug-av.o

clean:
	rm -f *.o discover
