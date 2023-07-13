all:
	make sys
	make C
sys:
	gcc -o sysFunc sysFunc.c

C:
	gcc -o cFunc cFunc.c
.PHONY:test
test:
	make test_c
	make test_sys
.PHONY:test_c
test_c:
	touch test_c
	./cFunc m p cFunc.c test_c; tr m p < cFunc.c > tmp ; diff -s tmp test_c
.PHONY:test_sys
test_sys:
	touch test_sys
	./sysFunc m p sysFunc.c test_sys; tr m p < sysFunc.c > tmp ; diff -s tmp test_sys

clean:
	rm -f cFunc sysFunc test tmp test_c test_sys