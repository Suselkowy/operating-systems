all:
	make displayFiles

.PHONY:displayFiles
displayFiles:
	gcc -o displayFiles displayFiles.c

make test:
	make test_displayFiles

.PHONY:test_displayFiles
test_displayFiles:
	./displayFiles

clean:
	rm -f displayFiles