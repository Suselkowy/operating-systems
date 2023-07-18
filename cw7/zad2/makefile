all:
	make server
	make client
	make hairdresser

.PHONY:server
server:
	gcc -o server server.c semaphores_helpers.c -lm -lrt

.PHONY:client
client:
	gcc -o client client.c semaphores_helpers.c -lrt

.PHONY:hairdresser
hairdresser:
	gcc -o hairdresser hairdresser.c semaphores_helpers.c -lrt

clean:
	rm -f server
	rm -f client
	rm -f hairdresser