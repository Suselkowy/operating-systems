all:
	make sys
	make C
sys:
	gcc -o sysFunc sysFunc.c

C:
	gcc -o cFunc cFunc.c
.PHONY:test
test:
	touch test
	./sysFunc m p sysFunc.c test; tr m p < sysFunc.c > tmp ; diff -s tmp test

clean:
	rm -f cFunc sysFunc test tmp