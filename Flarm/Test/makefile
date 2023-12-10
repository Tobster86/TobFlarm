CC = gcc
CFLAGS = -g -Wall -I..

SRC_DIR = .
OUT_DIR = ..

SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(OUT_DIR)/*.c)
OBJ_FILES = $(patsubst %.c,%.o,$(SRC_FILES))

all: test

test: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f test $(OBJ_FILES)

