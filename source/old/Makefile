all: saucer.o ship.o game.o rocket.o
	gcc -o saucer saucer.o ship.o game.o rocket.o -lcurses -pthread -Wall -I.
saucer.o: saucer.c
	gcc -c saucer.c -lcurses -pthread -Wall -I.
ship.o: ship.c
	gcc -c ship.c -lcurses -pthread -Wall -I.
game.o: game.c
	gcc -c game.c -lcurses -pthread -Wall -I.
rocket.o: rocket.c
	gcc -c rocket.c -lcurses -pthread -Wall -I.
	
old: 
	gcc -o saucer old_saucer.c -Wall -lcurses -pthread
	
	
