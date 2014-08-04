buildrun:
	gcc main.c utility.c bitmap.c -lglut -lGL -lGLU -lm -o fin -Wall
	./fin
build:
	gcc main.c utility.c bitmap.c -lglut -lGL -lGLU -lm -o fin -Wall
run:
	./fin
clear:
	rm fin