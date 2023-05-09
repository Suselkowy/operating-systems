all:
	make server
	make client
.PHONY:server
server:
	gcc -o server server.c -lm

.PHONY:client
client:
	gcc -o client client.c

clean:
	rm -f server
	rm -f client