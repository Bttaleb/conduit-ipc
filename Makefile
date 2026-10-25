CC      = clang
CFLAGS  = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined

test_io: src/io.c src/io.h src/test_io.c
	$(CC) $(CFLAGS) src/io.c src/test_io.c -o test_io

test: test_io
	./test_io

clean:
	rm -rf test_io test_io.dSYM

.PHONY: test clean
