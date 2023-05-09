all:
	make displayFiles

.PHONY:displayFiles
displayFiles:
	gcc -o displayFiles displayFiles.c

test1:
	./displayFiles . \#include

test2:
	./displayFiles .. \#include

test3:
	./displayFiles ../.. asdf

test4:
	./displayFiles /root asdf

test5:
	./displayFiles ~ asdf
	

clean:
	rm -f displayFiles

