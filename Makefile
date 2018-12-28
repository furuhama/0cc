0cc: 0cc.c
		gcc-8 0cc.c -o 0cc

test: 0cc
		./test.sh

clean:
		rm -f 0cc tmp*
