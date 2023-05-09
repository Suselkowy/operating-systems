all:
	make namedStream
	make calc

.PHONY:namedStream
namedStream:
	gcc namedStream.c -lm -o namedStream

.PHONY:calc
calc:
	gcc calc.c -lm -o calc 

clean:
	rm -f namedStream
	rm -f calc
	rm -f potok

