CC = clang
CFLAGS = -g -Wall

.PHONY: all clean

all: app test

app:
	$(CC) $(CFLAGS) -o ./build/todo_app src/main.c src/request_handling.c -lmicrohttpd -lsodium -lsqlite3 -lhiredis -lcurl -lrhonabwy

runa:
	./build/todo_app 8080

test:
	$(CC) $(CFLAGS) -DTEST_BUILD -o ./build/test_runner ./tests/tests.c ./src/request_handling.c ./lib/munit/munit.c -Iinclude -Ilib/munit -lmicrohttpd -lsodium -lsqlite3 -lhiredis -lcurl -lrhonabwy

runt:
	./build/test_runner

clean:
	rm -f ./build/todo_app ./build/test_runner
