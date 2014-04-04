#include	<stdio.h>
#include	<curses.h>
#include	<ncurses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<ship.h>
#include	<game.h>
#include	<rocket.h>

/*Lock used for locking curses access */
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

/*Variables*/
int 		turretCol;

/**
* Game set-up
*/
void setupGame()
{
	//Initialize ships
	initShips();
	//Initialize the rockets
	initRockets();
	
	/* set up curses */
	initscr();
	crmode();
	noecho();
	clear();
	keypad(stdscr, TRUE);
	
	turretCol = COLS / 2;
	
	printInfo();
	mvprintw(LINES-2,turretCol,TURRET);
}

void stopGame(){
	stopAllSaucers();
	endwin();
}

pthread_mutex_t getCursesLock(){
	return mx;
}

int getTurretPosition(){
	return turretCol;
}

void incrementTurret(){
	turretCol++;
}

void decrementTurret(){
	turretCol--;
}

/**
* Draws the Turret to the screen
*/
void drawTurret(){
	pthread_mutex_lock(&mx);
	move( LINES-2, turretCol-1);	/* can call curses	*/
	addch(' ');			/* at a the same time	*/
	addstr(TURRET);		/* Since I doubt it is	*/
	addch(' ');			/* reentrant		*/
	move(LINES-1,COLS-1);	/* park cursor		*/
	refresh();
	pthread_mutex_unlock(&mx);
}


/**
* Prints the game info at the bottom of the screen
*/
void printInfo(){
	char temp[1024];
	snprintf(temp, 1024, "Quit: 'Q' | Move: 'a'&'d' | Fire: 'space' | ESCAPED: %d | ROCKETS: %03d", getNumEscaped(), getNumRockets());
	pthread_mutex_lock(&mx);	/* only one thread	*/
	move( LINES -1, 0 );	/* can call curses	*/
	addstr( temp );		
	move(LINES-1,COLS-1);	/* park cursor		*/
	refresh();			/* and show it		*/
	pthread_mutex_unlock(&mx);	/* done with curses	*/
}
