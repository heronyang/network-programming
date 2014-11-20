all:
	gcc delayclient.c -o delayclient

run:
	for n in 1 3 4 ; do \
		echo run test $$n... ; \
		./delayclient localhost 33917 test$$n.txt > out/$$n.txt ; \
		done

