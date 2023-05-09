all:
	make displayFiles

.PHONY:displayFiles
displayFiles:
	gcc -o displayFiles displayFiles.c

make test:
	make create_test 
	make test_displayFiles

make create_test:
	mkdir empty
	mkdir semi-full
	cd semi-full; for number in 1 2 3 4 5; do \
    touch f$$number ; \
	done
	mkdir full
	cd full;for number in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do \
    touch f$$number ; \
	done
	
.PHONY:test_displayFiles
test_displayFiles:
	./displayFiles empty
	./displayFiles semi-full
	./displayFiles full

clean:
	rm -f displayFiles
	rm -rf full empty semi-full

