CC=gcc
OBJ=server.o
CFLAG=-g -Wall

PORT=33917

server: $(OBJ)
	$(CC) -o $@ $^
	rm *.o

%.o: %.c
	$(CC) $(CFLAG) -c -o $@ $<

run_server:
	@./server

run_client:
	@nc localhost $(PORT)

clean:
	rm server *.o

client:
	nc nplinux1.cs.nctu.edu.tw $(PORT)
