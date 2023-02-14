CC = gcc
LINK = mpicc

OMPFLAGS = -fopenmp
OMPLIBS = -lgomp

CFLAGS = -g -std=gnu99 -I. -Wall $(OMPFLAGS)
LDLIBS = $(OMPLIBS) -lm
INCLUDES = /usr/include/mpich

all: combined 

SRCS := $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) -c -v $(CFLAGS) -I$(INCLUDES) $(OMPFLAGS) $< -o $@

combined: $(OBJS) 
	$(LINK) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

clean:
	rm -rf *.o *.dSYM combined