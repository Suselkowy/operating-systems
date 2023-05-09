all:
	make sigTest
	make justRise
.PHONY:sigTest
sigTest:
	gcc -o sigTest sigTest.c
.PHONY:justRise
justRise:
	gcc -o justRise justRise.c

make test:
	rm -f raport.txt
	touch raport.txt

	./sigTest ignore >> raport.txt
	echo "" >> raport.txt

	./sigTest handler >> raport.txt
	echo "" >> raport.txt

	./sigTest mask >> raport.txt
	echo "" >> raport.txt

	./sigTest pending >> raport.txt
	echo "" >> raport.txt

clean:
	rm -f sigTest
	rm -f justRise
	rm -f raport.txt

