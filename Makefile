CC=gcc
LIBS=`curl-config --libs` `pkg-config --libs json-c`
CFLAGS=-march=native -O2 `pkg-config --cflags json-c`

all:
	$(CC) $(LIBS) $(CFLAGS) *.c -o bot

clean:
	rm -f bot
