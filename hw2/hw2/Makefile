CC=gcc
OBJ=main.o pipe.o fork_exec.o client.o broadcast.o
CFLAG=-g -Wall

PORT=33917

main: $(OBJ)
	$(CC) -o $@ $^
	cd src && $(MAKE)
	rm *.o

%.o: %.c
	$(CC) $(CFLAG) -c -o $@ $<

run_server:
	@./main

run_client:
	@nc localhost $(PORT)

clean:
	cd src && $(MAKE) clean
	rm main *.o

client:
	nc nplinux1.cs.nctu.edu.tw $(PORT)
