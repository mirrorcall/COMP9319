# Makefile

MODE = stable

CC = gcc
CFLAGS = -Wall

ifeq ($(MODE), dev)
    CFLAGS += -g -D DEBUG
else ifeq ($(MODE), stable)
	CFLAGS += -O3 -Wno-unused-result
else
    $(PANIC: error compile-time flag.)
endif

# bwtencode components
BECSRC = bwtencode.c futil.c
BEOBJ  = $(BECSRC:.c=.o)
# bwtsearch components
BSCSRC = bwtsearch.c futil.c
BSOBJ  = $(BSCSRC:.c=.o)
HSRC = futil.h

%o:%c $(HSRC)
	$(CC) $(CFLAGS) -c $<

# Special targets
.PHONY: clean

# Target rules
all: bwtencode bwtsearch
bwtencode: $(BEOBJ)
	$(CC) $(CFLAGS) -o bwtencode $(BEOBJ)

bwtsearch: $(BSOBJ)
	$(CC) $(CFLAGS) -o bwtsearch $(BSOBJ)

clean:
	rm -f $(BEOBJ) $(BSOBJ)