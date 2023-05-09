all:
	make big_test

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

create_test:
	echo "init 50" > test.txt
	echo "count main.c" >> test.txt
	echo "count libtextlibrary.c" >> test.txt
	echo "count libtextlibrary.h" >> test.txt

	echo "show 0" >> test.txt
	echo "show 1" >> test.txt
	echo "show 2" >> test.txt

	echo "delete 0" >> test.txt
	echo "delete 1" >> test.txt
	echo "delete 2" >> test.txt
	
	echo "destroy" >> test.txt
	echo "exit" >> test.txt


test:
	./static < test.txt > results_static$(O_FLAG) 2>/dev/null
	./shared < test.txt > results_shared$(O_FLAG) 2>/dev/null
	./dynamic < test.txt > results_dynamic$(O_FLAG) 2>/dev/null


small_test:
	make compile
	make create_test
	make test

big_test:
	make create_test

	make compile O_FLAG="-O"
	make test O_FLAG="_O"

	make compile O_FLAG="-O1"
	make test O_FLAG="_O1"

	make compile O_FLAG="-O2"
	make test O_FLAG="_O2"

	make compile O_FLAG="-O3"
	make test O_FLAG="_O3"

	make compile O_FLAG="-O0"
	make test O_FLAG="_O0"

	make compile O_FLAG="-Os"
	make test O_FLAG="_Os"

clean:
	rm -f result* test.txt  *.o *.so *.a shared static dynamic
