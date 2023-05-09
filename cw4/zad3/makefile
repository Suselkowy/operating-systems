all:
	make sender
	make catcher

.PHONY:sender
sender:
	gcc -o sender sender.c

.PHONY:catcher
catcher:
	gcc -o catcher catcher.c

clean:
	rm -f sender
	rm -f catcher

