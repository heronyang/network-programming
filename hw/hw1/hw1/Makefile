CC=gcc
OBJ=main.o

PORT=33916

main: $(OBJ)
	$(CC) -o $@ $^
	rm *.o

%.o: %.c
	$(CC) -c -o $@ $<

run_server:
	@./main

run_client:
	@nc localhost $(PORT)

clean:
	rm main *.o
