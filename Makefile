CC = gcc
CFLAGS = -std=gnu99 -pipe -O2 -pthread -Wall -fPIC -D_FILE_OFFSET_BITS=64
INCLUDE = -I/usr/include/python2.7 -I/opt/local/include

LIBS = -L/usr/lib/python2.7 -L/opt/local/lib
LDLIBS = -lfftw3 -lfftw3_threads -lpthread -lm -pipe
OSXLD = -bundle -flat_namespace -undefined suppress # OS X Linker Flags
POSLD = -shared # POSIX Linker Flags
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Darwin)
	LDFLAGS = $(OSXLD) $(LDLIBS)
else
	LDFLAGS = $(POSLD) $(LDLIBS)
endif

#Sources
SOURCE_VLFD = vlf_decimate.c
SOURCE_VLFF = vlf_fft.c 

#OBJECTS = $(SOURCES:.c=.o) 

#Executables
VLFDEC = vlfdec
VLF_FFT = vlf_fft

all: $(VLFDEC) $(VLF_FFT)

$(VLFDEC):
	$(CC) $(SOURCE_VLFD) -o $(VLFDEC) $(LDFLAGS) $(LIBS)

$(VLF_FFT):
	$(CC) $(SOURCE_VLFF) -o $(VLF_FFT) $(CFLAGS) $(LDFLAGS) $(LIBS)

#.c.o:
#	$(CC) -o $@ $(CFLAGS) $(INCLUDE) -c $<

clean:
	rm -f *.o $(VLFDEC) $(VLF_FFT)

#install:
#	cp _coreFFT.so coreFFT.py /usr/ggse/lib/
#	cp core-fft.py /usr/ggse/bin/
