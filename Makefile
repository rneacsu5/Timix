build:
	gcc main.c utility.c -lglut -lGL -lGLU -lm -o fin -Wall
run:
	./fin
buildrun:
	gcc main.c utility.c -lglut -lGL -lGLU -lm -o fin -Wall
	./fin
clear:
	rm fin