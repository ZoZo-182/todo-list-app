CC = clang
CFLAGS = -g -Wall

.PHONY: all clean

all: app test

app:
	$(CC) $(CFLAGS) -o ./build/todo_app src/main.c src/user_creds.c -lmicrohttpd -lsodium -lsqlite3

runa:
	./build/todo_app 8080

test:
	$(CC) $(CFLAGS) -o ./build/test_runner ./tests/tests.c ./src/user_creds.c ./lib/munit/munit.c -Iinclude -Ilib/munit -lmicrohttpd -lsodium -lsqlite3

runt:
	./build/test_runner 8080

clean:
	rm -f ./build/todo_app ./build/test_runner


