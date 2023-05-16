CC=g++
CFLAGS=-I. -Wall -fsanitize=undefined -fsanitize=address

.PHONY = run clean all archive

build:	client

all: build run

client: src/*.cc client.cc
	$(CC) $(CFLAGS) -o $@ $^

run: client
	./client

archive: clean
	zip -r Rotari_Corneliu_323CD_Tema3PC.zip *

clean:
	rm -f *.o client Rotari_Corneliu_323CD_Tema3PC.zip
