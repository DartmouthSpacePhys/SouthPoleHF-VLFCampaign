CC = gcc
CFLAGS = -std=gnu99 -pipe -O2 -Wall
# -I./include
LDFLAGS = -pipe -Wall
LDLIBS = -lcomedi -lm

SOURCES = mmap.c common.c
OBJECTS = $(SOURCES:.c=.o)
EXEC = acq_c

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	rm *.o $(EXEC)

install:
	cp acq_c /usr/comedi/bin/
