build:
	gcc main.c utility.c -lglut -lGL -lGLU -lm -o fin -Wall
run:
	./fin
clear:
	rm fin