CC=clang
cflags=

all:client server

writen.o:writen.c
	$(CC) -o $@ -c $<
readline.o:readline.c
	$(CC) -o $@ -c $<
error.o:error.c
	$(CC) -o $@ -c $<

client: client.c str_cli.c writen.o readline.o error.o
	$(CC) $(cflags) -o $@ $^
server: server.c error.o writen.o
	$(CC) $(cflags) -o $@ $^
clean:
	rm client server *.o
