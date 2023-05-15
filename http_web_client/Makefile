CC=g++
CFLAGS=-I. -Wall -fsanitize=undefined -fsanitize=address

client: src/*.cc headers/*.hh client.cc
	$(CC) $(CFLAGS) -o $@ $^

run: client
	./client

clean:
	rm -f *.o client
