CC = gcc
CCFLAGS = 

TGT = caboose
SRCS = $(wildcard src/*.c)

.PHONY: all
all:
	$(CC) $(CCFLAGS) -o $(TGT) $(SRCS)