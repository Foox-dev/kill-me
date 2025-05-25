Sure, here's the contents for the file: /raylib-game/raylib-game/Makefile

CC = gcc
CFLAGS = -Wall -Wextra -I./src -I./src/game -I./src/network -I./src/utils -lraylib -lm -lpthread -ldl -lrt -lX11
SRC = src/main.c src/game/player.c src/game/editor.c src/game/block.c src/network/network.c src/utils/debug.c
OBJ = $(SRC:.c=.o)
EXEC = raylib-game

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)