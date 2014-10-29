CC=g++

all: $(patsubst %.cpp, %, $(wildcard *.cpp))
	ln -fs /bin/cat ../ras/bin/
	ln -fs /bin/ls ../ras/bin/
	cp noop ../ras/bin/
	cp number ../ras/bin/
	cp removetag ../ras/bin/
	cp removetag0 ../ras/bin/

clean:
	rm noop number removetag removetag0
	rm ls cat
