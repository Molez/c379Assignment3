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
#include	<ncurses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<game.h>
#include	<ship.h>
#include	<rocket.h>

/**
* Main!
*/
int main(int ac, char *av[])
{
	int	       c;			/* user input			*/
	int	     i;

	setup();
	/* process user input */
	while(1) {
		c = getch();
		if ( c == 'Q' ) break;
		
		if ( c == 'a' || c == KEY_LEFT ){
			if(getTurretCol() > 1){
				decrementTurret();
			}
			drawTurret();
		}
		if ( c == 'd' || c == KEY_RIGHT ){
			if(getTurretCol() < (COLS - 1)){
				incrementTurret();
			}
			drawTurret();
		}
		if( c == ' '){
			if(numRockets > 0){
				spawnRocket();
				pthread_mutex_lock(&currentRockets);
				numRockets--;
				pthread_mutex_unlock(&currentRockets);
				printInfo();
			}
		}
	}
	/* cancel all the threads */
	pthread_mutex_lock(&mx);
	for (i=0; i<MAXSAUCERS; i++ )
		pthread_cancel(pSaucers[i]);
	endwin();
	return 0;
}