CC=gcc
CFLAGS=-Wall -O1 -g

all: bw

bw: error.o bitwise.o version.h

%.o: %.h

FORCE:
version.h: FORCE
	echo '#define BW_VERSION "1.0"' > version.h

.PHONY: clean
clean:
	rm -f error.o bitwise.o version.h bw
