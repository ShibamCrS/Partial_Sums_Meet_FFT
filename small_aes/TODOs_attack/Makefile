CC=gcc
OPT= -O2 -g -fsanitize=address
LIB=-lm -lpthread

UTL_DIC="../utility/"
SRC_DIC=.

UTL=$(shell ls ../utility/*.c)
SRC=$(shell ls *.c)
SRCS=$(UTL) $(SRC)  

all: main
main : main.c 
	@mkdir -p ../data
	$(CC) $(OPT) -o main $(SRCS) $(LIB)

.PHONY: clean
clean:
	rm -f main

