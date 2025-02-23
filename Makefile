CC = gcc
CFLAGS = -Wall -Iinclude -Isrc/math -Isrc/utils -Ilib

SRCS = src/main.c src/math/arithmetic.c src/utils/logger.c lib/extra.c
OBJS = src/main.o src/math/arithmetic.o src/utils/logger.o lib/extra.o

TARGET = callgraph_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

src/main.o: src/main.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/math/arithmetic.o: src/math/arithmetic.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/utils/logger.o: src/utils/logger.c
	$(CC) $(CFLAGS) -c -o $@ $<

lib/extra.o: lib/extra.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
