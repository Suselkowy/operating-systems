all:
	make server
	make client
	make hairdresser

.PHONY:server
server:
	gcc -o server server.c -lm

.PHONY:client
client:
	gcc -o client client.c

.PHONY:hairdresser
hairdresser:
	gcc -o hairdresser hairdresser.c

clean:
	rm -f server
	rm -f client
	rm -f hairdresser