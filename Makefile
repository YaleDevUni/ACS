.phony all:
all: ACS

ACS: main.c
	gcc -Wall -pthread -o ACS main.c

.PHONY clean:
clean:
	-rm -rf *.o *.exe
