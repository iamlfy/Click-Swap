#!/bin/bash

# Create bin directory if it doesn't exist
if [ ! -d "./bin" ]; then
    mkdir bin
fi

# Build the game
# gcc -O2 -DNODEBUG ./src/main.c ./src/ball.c ./src/paddle.c -o ./bin/game \
gcc -ggdb -pedantic-errors -Werror -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion \
	./src/main.c -o ./bin/game \
	-I./include -I/opt/homebrew/Cellar/raylib/5.5/include \
	-L/opt/homebrew/Cellar/raylib/5.5/lib -lraylib \
	&& echo "Build successful! Run with: ./bin/game" \
	&& ./bin/game
