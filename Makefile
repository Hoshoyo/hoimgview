all:
	gcc -Iinclude -g src/*.c -o bin/hobatch -lm -lGL -lglfw lib/libfreetype.a
