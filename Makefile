all:
	clang -o gotobin gotobin.c -O3

install:
	mv gotobin /usr/bin/

clean:
	rm gotobin
