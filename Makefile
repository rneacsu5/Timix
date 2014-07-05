build:
	gcc main.c -lglut -lGL -lGLU -o fin -Wall
run:
	./fin
clear:
	rm fin