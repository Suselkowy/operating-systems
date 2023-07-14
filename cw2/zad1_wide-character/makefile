all:
	make C
C:
	gcc -o cFunc cFunc.c
.PHONY:test
test:
	make test_c
.PHONY:test_c
test_c:
	touch test_c
	./cFunc m p cFunc.c test_c; tr m p < cFunc.c > tmp ; diff -s tmp test_c
clean:
	rm -f cFunc sysFunc test tmp test_c test_sys