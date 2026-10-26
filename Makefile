CC      = clang
CFLAGS  = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined

test_io: src/io.c src/io.h src/test_io.c
	$(CC) $(CFLAGS) src/io.c src/test_io.c -o test_io

test_protocol: src/io.c src/protocol.c src/protocol.h src/test_protocol.c
	$(CC) $(CFLAGS) src/io.c src/protocol.c src/test_protocol.c -o test_protocol

test: test_io test_protocol
	./test_io
	./test_protocol

clean:
	rm -rf test_io test_io.dSYM
	rm -rf test_protocol test_protocol.dSYM

.PHONY: test clean
