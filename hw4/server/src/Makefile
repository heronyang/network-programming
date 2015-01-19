CC=g++

all: $(patsubst %.cpp, %, $(wildcard *.cpp))
	cp /bin/cat ../ras_template/bin/
	cp /bin/ls ../ras_template/bin/
	cp noop ../ras_template/bin/
	cp number ../ras_template/bin/
	cp removetag ../ras_template/bin/
	cp removetag0 ../ras_template/bin/
	cp delayedremovetag ../ras_template/bin/

clean:
	rm noop number removetag removetag0 delayedremovetag
	rm ls cat
