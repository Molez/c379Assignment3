#CC=gcc
#CFLAGS=-Wall -lcurses -pthread
#OBJS=rocket.o game.o ship.o saucer.o
#PROG=fullgame

#all: $(PROG)
fullgame: rocket.o game.o ship.o saucer.o
	gcc -Wall -lcurses -pthread rocket.o game.o ship.o saucer.o -o saucer -I .
rocket.o: rocket.c
	gcc -Wall -lcurses -pthread -c rocket.c -I .
game.o: game.c
	gcc -Wall -lcurses -pthread -c game.c -I .
ship.o: ship.c
	gcc -Wall -lcurses -pthread -c ship.c -I .
saucer.o: saucer.c
	gcc -Wall -lcurses -pthread -c saucer.c -I .

