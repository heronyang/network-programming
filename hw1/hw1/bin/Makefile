CC=g++

all: $(patsubst %.cpp, %, $(wildcard *.cpp))

%.out: %.cpp Makefile
	$(CC) $< -o $@
