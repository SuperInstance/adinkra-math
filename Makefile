CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -Iinclude
SRCDIR = src
INCDIR = include

SRCS = $(SRCDIR)/adinkra.c $(SRCDIR)/supersymmetry.c $(SRCDIR)/glyph.c \
       $(SRCDIR)/encoding.c $(SRCDIR)/topology.c $(SRCDIR)/adinkra_api.c

all: libadinkra.a example

libadinkra.a: $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCDIR)/adinkra.c -o $(SRCDIR)/adinkra.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/supersymmetry.c -o $(SRCDIR)/supersymmetry.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/glyph.c -o $(SRCDIR)/glyph.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/encoding.c -o $(SRCDIR)/encoding.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/topology.c -o $(SRCDIR)/topology.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/adinkra_api.c -o $(SRCDIR)/adinkra_api.o
	ar rcs $@ $(SRCDIR)/*.o

test: tests/test_all.c libadinkra.a
	$(CC) $(CFLAGS) tests/test_all.c -L. -ladinkra -lm -o tests/test_runner
	./tests/test_runner

example: examples/example.c libadinkra.a
	$(CC) $(CFLAGS) examples/example.c -L. -ladinkra -lm -o examples/example_runner

clean:
	rm -f $(SRCDIR)/*.o libadinkra.a tests/test_runner examples/example_runner examples/*.svg

.PHONY: all test clean example
