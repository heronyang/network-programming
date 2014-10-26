CC=gcc
OBJ=main.o
CFLAG=-g -Wall

PORT=33916

main: $(OBJ)
	$(CC) -o $@ $^
	rm *.o

%.o: %.c
	$(CC) $(CFLAG) -c -o $@ $<

run_server:
	@./main

run_client:
	@nc localhost $(PORT)

clean:
	rm main *.o
