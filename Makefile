CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline

all: decode

decode: decode.c zerg.o tree.o graph.o
	$(CC) $(CFLAGS) obj/zerg.o obj/tree.o obj/graph.o decode.c -o bin/$@ -lm

zerg.o: lib/zerg.c
	$(CC) $(CFLAGS) $< -c -o obj/zerg.o -lm

tree.o: lib/tree.c
	$(CC) $(CFLAGS) $< -c -o obj/tree.o -lm

graph.o: lib/graph.c
	$(CC) $(CFLAGS) $< -c -o obj/graph.o -lm

debug: CFLAGS += -g -fstack-usage
debug: all

clean:
	rm -f bin/* obj/* *.su
