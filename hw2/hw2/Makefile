CC=gcc
OBJ=main.o pipe.o fork_exec.o client.o broadcast.o client_name.o fifo.o fifo_lock.o
CFLAG=-g -Wall

PORT=33917

main: $(OBJ)
	$(CC) -o $@ $^
	cd src && $(MAKE)
	rm *.o

%.o: %.c
	$(CC) $(CFLAG) -c -o $@ $<

run_server:
	rm -r ras
	cp -r ras_template ras
	@./main

run_client:
	@nc localhost $(PORT)

clean:
	cd src && $(MAKE) clean
	rm main *.o

client:
	nc nplinux1.cs.nctu.edu.tw $(PORT)
