all:
	make server
	make client
	make hairdresser

.PHONY:server
server:
	gcc -o server server.c -lm -lrt

.PHONY:client
client:
	gcc -o client client.c -lrt

.PHONY:hairdresser
hairdresser:
	gcc -o hairdresser hairdresser.c -lrt

clean:
	rm -f server
	rm -f client
	rm -f hairdresser