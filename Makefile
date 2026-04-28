CC=gcc-14
CFLAGS=-Wall -std=c2x
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

0cc: $(OBJS)
		$(CC) -o 0cc $(OBJS) $(LDFLAGS)

$(OBJS): 0cc.h

test: 0cc
		./0cc -test
		./test.sh

docker-build:
		docker build --platform linux/amd64 -t 0cc .

docker-test: docker-build
		docker run --rm --platform linux/amd64 0cc make test

clean:
		rm -f 0cc tmp* *.o *~
