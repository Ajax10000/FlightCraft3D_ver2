fc3D: airplane.o globals.o graphics.o main.o math.o physics.o specialEffects.o terrain.o trees.o
	gcc -o fc3D airplane.o globals.o graphics.o main.o math.o physics.o specialEffects.o terrain.o trees.o -lm -lSDL2 -lGL -lGLU  

airplane.o: airplane.c airplane.h globals.c globals.h
	gcc -c airplane.c 

globals.o: globals.c globals.h 
	gcc -c globals.c 

graphics.o: graphics.c graphics.h globals.c globals.h
	gcc -c graphics.c 

main.o: main.c globals.c globals.h 
	gcc -c main.c 

math.o: math.c math.h globals.h
	gcc -c math.c 

physics.o: physics.c physics.h globals.h
	gcc -c physics.c 

specialEffects.o: specialEffects.c specialEffects.h globals.h
	gcc -c specialEffects.c 

terrain.o: terrain.c terrain.h 
	gcc -c terrain.c 

trees.o: trees.c trees.h globals.h
	gcc -c trees.c 

clean: 
	rm *.o 
	rm fc3D 

