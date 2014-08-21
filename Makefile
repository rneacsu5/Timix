buildrun:
	rm -rf bin
	mkdir bin
	gcc -g src/main.c src/utility.c src/lobjder.c src/motion.c -lglut -lGL -lGLU -lGLEW -lm -Linclude -Wl,-rpath=./include/ -lbass64 -o bin/fin -Wall
	./bin/fin
build:
	rm -rf bin
	mkdir bin
	gcc -g src/main.c src/utility.c src/lobjder.c src/motion.c -lglut -lGL -lGLU -lGLEW -lm -Linclude -Wl,-rpath=./include/ -lbass64 -o bin/fin -Wall
run:
	./bin/fin
clear:
	rm -rf bin
