CC=gcc
cflags=
localhost=127.0.0.1
port=9877
max_number_of_clients=2
U=chaofan

all:client server
msg.o: msg.c
	$(CC) -o $@ -c $<
error.o:error.c
	$(CC) -o $@ -c $<

client: team1_client.c str_cli.c error.o msg.o
	$(CC) $(cflags) -o $@ $^
server: team1_server.c error.o msg.o
	$(CC) $(cflags) -o $@ $^
clean:
	rm client server *.o
s: server
	./server $(localhost) $(port) $(max_number_of_clients) 
c: client
	./client $(U) $(localhost) $(port)
