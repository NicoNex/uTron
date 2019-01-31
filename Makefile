CC=gcc
LIBS=`curl-config --libs` `pkg-config --libs json-c` -lpthread
CFLAGS=-march=native -O2 `pkg-config --cflags json-c`

all:
	$(CC) $(LIBS) $(CFLAGS) engine.c network.c test.c -o bot

clean:
	rm -f bot
