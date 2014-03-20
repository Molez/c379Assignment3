#ifndef ROCKET_H
#define ROCKET_H

/*Global Definitions*/
#define ROCKET "^"		/* The Rocket String*/

struct rocket{
	char *str;	/*The rocket String*/
	int row;	/*The row*/
	int col;	/*The column*/
	int delay;	/*The Speed of the rocket*/
	int dir;	/* +1 or -1*/
	int alive;	/*Indicates if this rocket is alive*/
};

/**
* Function prototypes
*/
void spawnRocket();
void	       *animateRocket();
void resetRocket(struct rocket * rocket);
void initRockets();
#endif