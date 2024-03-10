all: jokes

jokes: main.c
	clang -fcolor-diagnostics -fansi-escape-codes -g main.c -o jokes

clean:
	rm -f jokes
