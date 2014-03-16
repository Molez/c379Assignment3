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
#define MAXSAUCERS 500		/* An Obsurdly large max saucer count*/
#define MAXROCKETS 500		/* The maximum number of in flight rockets*/

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

pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

int main(int ac, char *av[])
{
	int	       c;			/* user input			*/
	pthread_t      pSaucers[MAXSAUCERS];	/* The saucer threads		*/
	pthread_t      pRockets[MAXROCKETS];	/* The rocket threds		*/
	struct saucer sProps[MAXSAUCERS];	/* Properties of Saucers	*/
	struct rocket rProps[MAXROCKETS];	/* Properties of Rockets	*/
	void	       *animate();		/* the function			*/
	int	       num_msg ;		/* number of strings		*/
	int	     i;

	/*
	if ( ac == 1 ){
		printf("usage: tanimate string ..\n"); 
		exit(1);
	}
	*/

	setup();

	/* create all the threads */
	for(i=0 ; i<num_msg; i++)
		if ( pthread_create(&thrds[i], NULL, animate, &props[i])){
			fprintf(stderr,"error creating thread");
			endwin();
			exit(0);
		}

	/* process user input */
	while(1) {
		c = getch();
		if ( c == 'Q' ) break;
		if ( c == ' ' )
			for(i=0;i<num_msg;i++)
				props[i].dir = -props[i].dir;
		if ( c >= '0' && c <= '9' ){
			i = c - '0';
			if ( i < num_msg )
				props[i].dir = -props[i].dir;
		}
	}

	/* cancel all the threads */
	pthread_mutex_lock(&mx);
	for (i=0; i<num_msg; i++ )
		pthread_cancel(thrds[i]);
	endwin();
	return 0;
}

int setup()
{
//	int num_msg = ( nstrings > MAXMSG ? MAXMSG : nstrings );
//	int i;

	/* assign rows and velocities to each string */
//	srand(getpid());
//	for(i=0 ; i<num_msg; i++){
//		props[i].str = strings[i];	/* the message	*/
//		props[i].row = i;		/* the row	*/
//		props[i].delay = 1+(rand()%15);	/* a speed	*/
//		props[i].dir = ((rand()%2)?1:-1);	/* +1 or -1	*/
//	}

	/*reset all saucers to default*/

	/*reset all rocklets to default*/
	
	/* set up curses */
	initscr();
	crmode();
	noecho();
	clear();
//	mvprintw(LINES-1,0,"'Q' to quit, '0'..'%d' to bounce",num_msg-1);

//	return num_msg;
}
/*Sets a saucer struct to default settings*/
void resetSaucer(struct saucer * saucer){

}

void resetRocket(struct rocket * rocket){

}

/* the code that runs in each thread */
void *animate(void *arg)
{
	struct propset *info = arg;		/* point to info block	*/
	int	len = strlen(info->str)+2;	/* +2 for padding	*/
	int	col = rand()%(COLS-len-3);	/* space for padding	*/

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

		if ( col <= 0 && info->dir == -1 )
			info->dir = 1;
		else if (  col+len >= COLS && info->dir == 1 )
			info->dir = -1;
	}
}
