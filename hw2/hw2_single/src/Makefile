CC=g++

all: $(patsubst %.cpp, %, $(wildcard *.cpp))
	cp /bin/cat ../ras/bin/
	cp /bin/ls ../ras/bin/
	cp noop ../ras/bin/
	cp number ../ras/bin/
	cp removetag ../ras/bin/
	cp removetag0 ../ras/bin/

clean:
	rm noop number removetag removetag0
	rm ls cat
