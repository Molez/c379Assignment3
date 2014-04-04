#include	<stdio.h>
#include	<curses.h>
#include	<ncurses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<rocket.h>
#include	<game.h>

/*Lock used for incrementing/decrementing the users rocket count*/
pthread_mutex_t currentRockets= PTHREAD_MUTEX_INITIALIZER;
/*Lock used for accessing any of the rocket data structures*/
pthread_mutex_t rockets = PTHREAD_MUTEX_INITIALIZER; 

/*Variables*/
struct rocket 	rProps[MAXROCKETS];	/* Properties of Rockets	*/
pthread_t      	pRockets[MAXROCKETS];	/* The rocket threds		*/
int numRockets = DEFAULTROCKETS;

/*Function Prototypes*/
void	       *animateRocket();
void resetRocket(struct rocket * rocket);

void initRockets(){
	int i;
	
	/*reset all rocklets to default*/
	for(i=0; i < MAXROCKETS; i++){
		resetRocket(&rProps[i]);
	}
}

/**
* Sets a rocket struct to default settings
*/
void resetRocket(struct rocket * rocket){
	rocket->str = ROCKET;
	rocket->row = (LINES - 3);
	rocket->col = 0;
	rocket->delay = 5;
	rocket->dir = -1;
	rocket->alive = -1;
}

int getNumRockets(){
	return numRockets;
}

void decrementRockets(){
	pthread_mutex_lock(&currentRockets);
	numRockets--;
	pthread_mutex_unlock(&currentRockets);
}

void spawnRocket(){
	int i;
	pthread_mutex_lock(&rockets);
		for(i=0; i < MAXROCKETS; i++){
			if(rProps[i].alive == -1){
				resetRocket(&rProps[i]);
				rProps[i].col = getTurretPosition();
				pthread_create(&pRockets[i], NULL, animateRocket, &rProps[i]);
				rProps[i].alive = 1;
				break;
			}
		}
	pthread_mutex_unlock(&rockets);
}

/**
* Code that runs each rocket on a thread
*/
void *animateRocket(void *arg)
{
	struct rocket *info = arg;		/* point to info block	*/
	int row = LINES - 3;
	pthread_mutex_t mx = getCursesLock();
	
	pthread_mutex_lock(&mx);	/* only one thread	*/
		   move( row, info->col );	/* can call curses	*/
		   addstr( info->str );		/* Since I doubt it is	*/
		   move(LINES-1,COLS-1);	/* park cursor		*/
		   refresh();			/* and show it		*/
	pthread_mutex_unlock(&mx);	/* done with curses	*/

	row += info->dir;
	
	while( 1 )
	{
		usleep(info->delay*TUNIT);

		pthread_mutex_lock(&mx);	/* only one thread	*/
		   move( row + 1, info->col );	/* can call curses	*/
		   addch(' ');			/* at a the same time	*/
		   move( row, info->col );	/* can call curses	*/
		   addstr( info->str );		/* Since I doubt it is	*/
		   move(LINES-1,COLS-1);	/* park cursor		*/
		   refresh();			/* and show it		*/
		pthread_mutex_unlock(&mx);	/* done with curses	*/

		/* move item to next column and check for bouncing	*/
		row += info->dir;

		if(row < 0){
			pthread_mutex_lock(&mx);	/* only one thread	*/
		   	move( row + 1, info->col );	/* can call curses	*/
		  	addch(' ');			/* at a the same time	*/ 
		  	move(LINES-1,COLS-1);	/* park cursor		*/
		  	refresh();			/* and show it		*/
			pthread_mutex_unlock(&mx);	/* done with curses	*/
			
			pthread_mutex_lock(&rockets);
			info->alive = -1;
			pthread_mutex_unlock(&rockets);
			pthread_exit(NULL);
		}
	}
}