all:
	clang -o gotobin gotobin.c -O3

install:
	cp gotobin /usr/bin/
	cat shellfunc.sh >> ~/.zshrc
	touch ~/.gotolist
	# cat shellfunc.sh >> ~/.bashrc

clean:
	rm gotobin
	rm ~/.gotolist
