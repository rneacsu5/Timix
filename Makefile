build:
	gcc test.c -lglut -lGL -lGLU -lm -o fin -Wall
run:
	./fin
clear:
	rm fin