all:
	gcc -Iinclude -g src/*.c -o bin/hoimgview -lm -lGL -lglfw lib/libfreetype.a
