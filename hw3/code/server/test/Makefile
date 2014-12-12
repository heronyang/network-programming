all:
	gcc delayclient.c -o delayclient

run:
	for n in 1 2 3 4 ; do \
		echo run test $$n... ; \
		./delayclient localhost 33917 test$$n.txt > out/$$n.txt ; \
		done

run_deploy:
	for n in 1 2 3 4 ; do \
		echo run test $$n... ; \
		./delayclient nplinux1.cs.nctu.edu.tw 33918 test$$n.txt > out_deploy/$$n.txt ; \
		done
