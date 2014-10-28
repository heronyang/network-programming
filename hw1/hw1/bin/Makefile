CC=g++

all: $(patsubst %.cpp, %, $(wildcard *.cpp))
	ln -s /bin/cat .
	ln -s /bin/ls .

%.out: %.cpp Makefile
	$(CC) $< -o $@
