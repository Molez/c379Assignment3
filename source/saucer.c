/*
 * tanimate.c: animate several strings using threads, curses, usleep()
 *
 *	bigidea one thread for each animated string
 *		one thread for keyboard control
 *		shared variables for communication
 *	compile	cc tanimate.c -lcurses -lpthread -o tanimate
 *	to do   needs locks for shared variables
 *	        nice to put screen handling in its own thread
 */

#include	<stdio.h>
#include	<curses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#define	SAUCER "<--->"		/* The saucer String*/
#define ROCKET "^"		/* The Rocket String*/
#define TURRET "|"		/* The Turret String*/
#define	TUNIT   20000		/* timeunits in microseconds */
#define MAXSAUCERS 20		/* An Obsurdly large max saucer count*/
#define MAXROCKETS 500		/* The maximum number of in flight rockets */

struct	saucer{
		char	*str;	/* the message */
		int	row;	/* the row     */
		int	delay;  /* delay in time units */
		int	dir;	/* +1 or -1	*/
		int 	alive;	/*Indicates if this saucer is alive*/
	};

struct rocket{
	char *str;	/*The rocket String*/
	int row;	/*The row*/
	int col;	/*The column*/
	int delay;	/*The Speed of the rocket*/
	int dir;	/* +1 or -1*/
	int alive;	/*Indicates if thie rocket is alive*/
};

pthread_t      pSaucers[MAXSAUCERS];	/* The saucer threads		*/
pthread_t      pRockets[MAXROCKETS];	/* The rocket threds		*/
struct saucer sProps[MAXSAUCERS];	/* Properties of Saucers	*/
struct rocket rProps[MAXROCKETS];	/* Properties of Rockets	*/
pthread_t	spawnSaucer;

/*LOCKS*/
/*-------------------------------------------------------------------*/
/*ALWAYS attempt to access them in the order they appear here to
* avoid race conditions */
/*-------------------------------------------------------------------*/
/*Lock used for locking curses access */
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
/*Lock used for accessing any of the saucer data structures*/
pthread_mutex_t saucers = PTHREAD_MUTEX_INITIALIZER; 

/**
* Function prototypes
*/
void resetSaucer(struct saucer * saucer);
void resetRocket(struct rocket * rocket);
void	       *animateSaucer();
void	       *saucerSpawn();

/**
* Main!
*/
int main(int ac, char *av[])
{
	int	       c;			/* user input			*/
	void	       *animateSaucer();		/* the function			*/
	int	     i;

	setup();

	/* create all the threads */
	/*
	for(i=0 ; i<num_msg; i++)
		if ( pthread_create(&thrds[i], NULL, animate, &props[i])){
			fprintf(stderr,"error creating thread");
			endwin();
			exit(0);
		}
	*/

	pthread_create(&spawnSaucer, NULL, saucerSpawn, NULL);
	/* process user input */
	while(1) {
		c = getch();
		if ( c == 'Q' ) break;
		}

	/* cancel all the threads */
	pthread_mutex_lock(&mx);
	for (i=0; i<MAXSAUCERS; i++ )
		pthread_cancel(pSaucers[i]);
	endwin();
	return 0;
}
/**
* Game set-up
*/
int setup()
{
	int i;
	/*reset all saucers to default*/
	for(i=0; i < MAXSAUCERS; i++){
		resetSaucer(&sProps[i]);
	}
	/*reset all rocklets to default*/
	for(i=0; i < MAXROCKETS; i++){
		resetRocket(&rProps[i]);
	}
	/* set up curses */
	initscr();
	crmode();
	noecho();
	clear();
//	mvprintw(LINES-1,0,"'Q' to quit, '0'..'%d' to bounce",num_msg-1);
}
/**
* Sets a saucer struct to default settings
*/
void resetSaucer(struct saucer * saucer){
	saucer->str = SAUCER;
	srand(time(NULL));
	saucer->row = (rand()%10);
	srand(time(NULL));
	//saucer->delay = 5+(rand()%10);
	saucer->delay = 2;
	saucer->dir = 1;
	saucer->alive = -1;
}
/**
* Sets a rocket struct to default settings
*/
void resetRocket(struct rocket * rocket){
	rocket->str = ROCKET;
	rocket->row = 0;
	rocket->col = 0;
	rocket->delay = 5;
	rocket->dir = 1;
	rocket->alive = -1;
}

/**
* Code that runs each saucer thread
*/
void *animateSaucer(void *arg)
{
	struct saucer *info = arg;		/* point to info block	*/
	int	len = strlen(info->str)+2;	/* +2 for padding	*/
	int	col = 0;				/* space for padding	*/

	while( 1 )
	{
		usleep(info->delay*TUNIT);

		pthread_mutex_lock(&mx);	/* only one thread	*/
		   move( info->row, col );	/* can call curses	*/
		   addch(' ');			/* at a the same time	*/
		   addstr( info->str );		/* Since I doubt it is	*/
		   addch(' ');			/* reentrant		*/
		   move(LINES-1,COLS-1);	/* park cursor		*/
		   refresh();			/* and show it		*/
		pthread_mutex_unlock(&mx);	/* done with curses	*/

		/* move item to next column and check for bouncing	*/
		col += info->dir;

		if(col >= COLS){
			pthread_mutex_lock(&saucers);
			info->alive = -1;
			pthread_mutex_unlock(&saucers);
			pthread_exit(0);
		}
	}
}

/**
* Code for the thread that spawns new saucers
*/
void *saucerSpawn(){
	int i;
	while(1)
	{
		for(i=0; i < MAXSAUCERS; i++){
			if(sProps[i].alive == -1){
				pthread_mutex_lock(&saucers);
				resetSaucer(&sProps[i]);
				pthread_create(&pSaucers[i], NULL, animateSaucer, &sProps[i]);
				sProps[i].alive = 1;
				pthread_mutex_unlock(&saucers);
				break;
			}
		}
		srand(time(NULL));
		usleep((250 + (rand()%250))*TUNIT);
	}
}
