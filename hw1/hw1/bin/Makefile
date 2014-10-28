CC=g++

all: $(patsubst %.cpp, %, $(wildcard *.cpp))
	ln -fs /bin/cat .
	ln -fs /bin/ls .
	ln -fs /usr/bin/printenv .

%.out: %.cpp Makefile
	$(CC) $< -o $@

clean:
	rm ls cat
	rm noop number removetag removetag0
