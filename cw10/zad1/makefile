all:
	make server
	make client
.PHONY:server
server:
	gcc -o server server_socket.c -lm

.PHONY:client
client:
	gcc -o client client_socket.c 

clean:
	rm -f server
	rm -f client