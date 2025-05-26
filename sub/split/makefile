CC = gcc
CFLAGS = -Wall -std=c99
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SOURCES = main.c submarine.c renderer.c
TARGET = submarine

all: $(TARGET)

$(TARGET): $(SOURCES)
		$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

clean:
		rm -f $(TARGET)

run: $(TARGET)
		./$(TARGET)

.PHONY: all clean run