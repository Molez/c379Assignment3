#include	<stdio.h>
#include	<curses.h>
#include	<ncurses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<game.h>
#include	<rocket.h>

/**
* Main!
*/
int main(int ac, char *av[])
{
	int	       c;			/* user input			*/
	
	setupGame();
	/* process user input */
	while(1) {
		c = getch();
		if ( c == 'Q' ) break;

		if ( c == 'a' || c == KEY_LEFT ){
			if(getTurretPosition() > 1){
				decrementTurret();
			}
			drawTurret();
		}
		if ( c == 'd' || c == KEY_RIGHT ){
			if(getTurretPosition() < (COLS - 1)){
				incrementTurret();
			}
			drawTurret();
		}
		if( c == ' '){
			if(getNumRockets() > 0){
				spawnRocket();
				decrementRockets();
				printInfo();
			}
		}
	}
	stopGame();
	return 0;
}