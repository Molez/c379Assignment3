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

/*Threads*/
pthread_t	spawnSaucer;
pthread_t	turretControl;

/*LOCKS*/
/*-------------------------------------------------------------------*/
/*ALWAYS attempt to access them in the order they appear here to
* avoid race conditions */
/*-------------------------------------------------------------------*/
/*Lock used for locking curses access */
extern pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
/*Lock used for accessing any of the saucer data structures*/
extern pthread_mutex_t saucers = PTHREAD_MUTEX_INITIALIZER;
/*Lock used for accessing any of the rocket data structures*/
extern pthread_mutex_t rockets = PTHREAD_MUTEX_INITIALIZER;
/*Lock used for incrementing the escaped saucers variable*/
extern pthread_mutex_t escaped = PTHREAD_MUTEX_INITIALIZER;
/*Lock used for incrementing/decrementing the users rocket count*/
extern pthread_mutex_t currentRockets = PTHREAD_MUTEX_INITIALIZER;

/*Variables*/
int 		turretCol;

void setup()
{
	int i;
	
	/*Initialize the ships and rockets*/
	initRockets();
	initShips();
	
	/* set up curses */
	initscr();
	crmode();
	noecho();
	clear();
	keypad(stdscr, TRUE);
	
	/*Print info to the bottom of the screen*/
	printInfo();
	
	/*Intialize the turret*/
	turretCol = COLS / 2;
	mvprintw(LINES-2,turretCol,TURRET);
	
	/*Create the saucer spawning thread*/
	pthread_create(&spawnSaucer, NULL, saucerSpawn, NULL);
}

void getTurretCol(){
	return turretCol();
}

void decrementTurret(){
	turretCol--;
}

void incrementTurret(){
	turretCol++;
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
	snprintf(temp, 1024, "Quit: 'Q' | Move: 'a'&'d' | Fire: 'space' | ESCAPED: %d | ROCKETS: %03d", numEscapted, numRockets);
	pthread_mutex_lock(&mx);	/* only one thread	*/
	move( LINES -1, 0 );	/* can call curses	*/
	addstr( temp );		
	move(LINES-1,COLS-1);	/* park cursor		*/
	refresh();			/* and show it		*/
	pthread_mutex_unlock(&mx);	/* done with curses	*/
}