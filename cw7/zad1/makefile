all:
	make server
	make client
	make hairdresser

.PHONY:server
server:
	gcc -o server server.c semaphores_helpers.c -lm

.PHONY:client
client:
	gcc -o client client.c semaphores_helpers.c

.PHONY:hairdresser
hairdresser:
	gcc -o hairdresser hairdresser.c semaphores_helpers.c

clean:
	rm -f server
	rm -f client
	rm -f hairdresser