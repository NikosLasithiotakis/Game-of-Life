CC = gcc
CFLAGS = -fopenmp -ansi -pedantic -Wall -std=c99
TARGET = game_of_life
SRC = game_of_life.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) output.txt