all:
	make static
	make shared
static:
	gcc -Wall -c libtextlibrary.c
	ar cr libtextlibrary.a libtextlibrary.o

shared:
	gcc -Wall -c -fPIC libtextlibrary.c
	gcc -Wall -shared -fPIC -o libtextlibrary.so libtextlibrary.o

clean:
	rm -f *.o *.so *.a
