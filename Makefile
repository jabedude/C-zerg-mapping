CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline

all: decode

decode: decode.c zerg.o
	$(CC) $(CFLAGS) decode.c obj/zerg.o -o bin/$@ -lm

zerg.o: lib/zerg.c
	$(CC) $(CFLAGS) $< -c -o obj/zerg.o -lm

pcap.o: lib/pcap.c
	$(CC) $(CFLAGS) $< -c -o obj/pcap.o -lm

debug: CFLAGS += -DDEBUG -g -fstack-usage
debug: all

clean:
	rm -f bin/* obj/* *.su
