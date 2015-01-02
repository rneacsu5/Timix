buildrun:
	rm -f bin/fin
	gcc -g src/main.c src/utility.c -lglut -lGL -lGLU -lGLEW -lm  -Llibs/Linux -Llibs/Linux/x64 -Wl,-rpath,./libs/Linux/:./libs/Linux/x64/ -lbass -o bin/fin -Wall
	./bin/fin
build:
	rm -f bin/fin
	gcc -g src/main.c src/utility.c -lglut -lGL -lGLU -lGLEW -lm -Llibs/Linux -Llibs/Linux/x64 -Wl,-rpath,./libs/Linux/:./libs/Linux/x64/ -lbass -o bin/fin -Wall
run:
	./bin/fin
clean:
	rm -f bin/fin