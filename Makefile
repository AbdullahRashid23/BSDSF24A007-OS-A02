CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = bin/ls
SRC = src/ls-v1.0.0.c

all: $(TARGET)

$(TARGET): $(SRC) | bin obj
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

bin obj:
	mkdir -p $@

clean:
	rm -rf bin obj

.PHONY: all clean
