CC=clang
cflags=
localhost=127.0.0.1
port=9877
max_number_of_clients=1027

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
s: server
	./server $(localhost) $(port) $(max_number_of_clients) 
c: client
	./client $(localhost)
