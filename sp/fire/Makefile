CC=i586-pc-msdosdjgpp-g++

all: main.o demo.exe clean

demo.exe: main.o
	$(CC) -o $@ main.o

main.o: main.cc
	$(CC) -c -o $@ main.cc -Ofast

clean:
	rm -f main.o
