CC=gcc
CFLAGS=-Wall -Wextra -g -Werror -Wno-error=unused-variable -fPIC -O2 -fsanitize=undefined -fsanitize=address
LIBFLAGS=-lm
SRC=src
SER=$(wildcard $(SRC)/server/*.c)
CLI=$(wildcard $(SRC)/client/*.c)

LIB=$(SRC)/lib
DLIBNAME=$(LIB)/libtcpapplication.so
LIB_SRC=$(wildcard $(LIB)/*.c)
LIB_OBJ=$(patsubst $(LIB)/%.c, obj/%.o, $(LIB))
LDFLAGS=-shared

.PHONY:	clean build server subscriber

build:	server subscriber

$(DLIBNAME):
	$(CC) $(CFLAGS) -c $(LIB_SRC)
	$(CC) $(LDFLAGS) -o $@ $(LIB_OBJ)

server:
	$(CC) $(CFLAGS) $(SER) $(LIB_SRC) -o server

subscriber:
	$(CC) $(CFLAGS) $(CLI) $(LIB_SRC) -o subscriber $(LIBFLAGS)

test:
	sudo python3 test.py 

clean:
	rm server subscriber