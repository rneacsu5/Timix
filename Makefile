buildrun:
	rm -f bin/fin
	gcc -g src/main.c src/utility.c -lglut -lGL -lGLU -lGLEW -lm -lbass -Llibs/Linux/lib/ -Llibs/Linux/lib/x64/ -Wl,-rpath,./libs/Linux/lib/:./libs/Linux/lib/x64/ -Ilibs/Linux/include/ -o bin/fin -Wall
	./bin/fin
build:
	rm -f bin/fin
	gcc -g src/main.c src/utility.c -lglut -lGL -lGLU -lGLEW -lm -lbass -Llibs/Linux/lib/ -Llibs/Linux/lib/x64/ -Wl,-rpath,./libs/Linux/lib/:./libs/Linux/lib/x64/ -Ilibs/Linux/include/ -o bin/fin -Wall
run:
	./bin/fin
clean:
	rm -f bin/fin
install:
	sudo apt-get install freeglut3-dev
	sudo apt-get install libglew1.5