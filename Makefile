CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

0cc: $(OBJS)
		gcc-8 -o 0cc $(OBJS) $(LDFLAGS)

$(OBJS): 0cc.h

test: 0cc
		./0cc -test
		./test.sh

clean:
		rm -f 0cc tmp* *.o *~
