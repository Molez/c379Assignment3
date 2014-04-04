#ifndef _ROCKET_H_
#define _ROCKET_H_

#define DEFAULTROCKETS 30	/* Number of starting Rockets*/
#define ROCKET "^"		/* The Rocket String*/
#define MAXROCKETS 200		/* The maximum number of in flight rockets */

struct rocket{
	char *str;	/*The rocket String*/
	int row;	/*The row*/
	int col;	/*The column*/
	int delay;	/*The Speed of the rocket*/
	int dir;	/* +1 or -1*/
	int alive;	/*Indicates if this rocket is alive*/
};

void initRockets();
int getNumRockets();
void decrementRockets();
int getNumRockets();
void spawnRocket();
#endif
