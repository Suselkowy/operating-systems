all:
	make compile

compile:
	make static_lib O_FLAG=$(O_FLAG)
	make shared_lib O_FLAG=$(O_FLAG)
	make static_exe O_FLAG=$(O_FLAG)
	make shared_exe O_FLAG=$(O_FLAG)
	make dynamic_exe O_FLAG=$(O_FLAG)

move_shered:
	sudo mv libtextlibrary.so /usr/lib

static:
	make static_lib
	make static_exe
shared:
	make shared_lib
	make shared_exe
dynamic:
	make shared_lib
	make dynamic_exe

shared_lib:
	gcc $(O_FLAG) -Wall -c -fPIC libtextlibrary.c
	gcc $(O_FLAG) -Wall -shared -fPIC -o libtextlibrary.so libtextlibrary.o
	make move_shered
static_lib:
	gcc $(O_FLAG) -Wall -c libtextlibrary.c
	ar cr libtextlibrary.a libtextlibrary.o

static_exe:
	gcc main.c $(O_FLAG) -Wall -o static -L. -ltextlibrary
shared_exe:
	gcc main.c $(O_FLAG) -Wall -o shared -L. -ltextlibrary
dynamic_exe:
	gcc main.c $(O_FLAG) -Wall -ldl -o dynamic -D DYNAMIC

clean:
	rm -f result* test.txt  *.o *.so *.a shared static dynamic
