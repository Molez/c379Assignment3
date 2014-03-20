#include	<stdio.h>
#include	<curses.h>
#include	<ncurses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<game.h>
#include	<ship.h>
#include	<rocket.h>

/*Variables*/
int		numEscapted;
pthread_t      	pSaucers[MAXSAUCERS];	/* The saucer threads		*/
struct saucer 	sProps[MAXSAUCERS];	/* Properties of Saucers	*/

/**
*Used to set-up all the variables and structures for rockets
*/
void initShips(){
	/*reset all saucers to default*/
	for(i=0; i < MAXSAUCERS; i++){
		resetSaucer(&sProps[i]);
	}
	
	numEscapted = 0;
}

/**
* Sets a saucer struct to default settings
*/
void resetSaucer(struct saucer * saucer){
	//saucer->str = SAUCER;
	strncpy(saucer->str, SAUCER, strlen(SAUCER));
	srand(time(NULL));
	saucer->row = (rand()%AIRSPACE);
	srand(time(NULL));
	saucer->delay = 2+(rand()%14);
	saucer->dir = 1;
	saucer->alive = -1;
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
			pthread_mutex_lock(&escaped);
				numEscapted++;
			pthread_mutex_unlock(&escaped);
			printInfo();
			pthread_exit(NULL);
		}
		/*Used to dynamically shrink the print string as we hit the end of the
		screen in order to ensure no wrap around*/
		if(col + len >= COLS){
			pthread_mutex_lock(&saucers);
			info->str[len-1] = '\0';
			pthread_mutex_unlock(&saucers);
			len = len - 1;
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
		pthread_mutex_lock(&saucers);
		for(i=0; i < MAXSAUCERS; i++){
			if(sProps[i].alive == -1){
				resetSaucer(&sProps[i]);
				pthread_create(&pSaucers[i], NULL, animateSaucer, &sProps[i]);
				sProps[i].alive = 1;
				break;
			}
		}
		pthread_mutex_unlock(&saucers);
		srand(time(NULL));
		usleep((100 + (rand()%100))*TUNIT);
	}
}
