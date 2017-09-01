CFLAGS = -Wall -Wextra -Werror -g

all: test_huff extract

test_huff: test_huff.c

extract: extract.cpp

clean:
	rm -f test_huff extract

test: test_huff
	./test_huff test
