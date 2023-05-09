all:
	make lib
	make reverse_1
	make reverse_2


lib:
	gcc -Wall -c libhelpers.c
	ar cr libhelpers.a libhelpers.o

.PHONY:reverse_1
reverse_1:
	gcc reverse1.c -Wall -o reverse1 -L. -lhelpers

.PHONY:reverse_2
reverse_2:
	gcc reverse2.c -Wall -o reverse2 -L. -lhelpers

.PHONY:test
test:
	rm -f pomiar_zad_2.txt
	make test_reverse_1
	make test_reverse_2

.PHONY:test_reverse_1
test_reverse_1:
	./reverse1 reverse1.c test
	./reverse1 test test2
	diff -s test2 reverse1.c

.PHONY:test_reverse_2
test_reverse_2:
	./reverse2 reverse1.c test
	./reverse2 test test2
	diff -s test2 reverse1.c

clean:
	rm -f *.a *.o reverse1 reverse2 test test2
