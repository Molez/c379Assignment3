#include	<stdio.h>
#include	<curses.h>
#include	<ncurses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#define	SAUCER "<--->"		/* The saucer String*/
#define SAUCER_LEN 5		/*The length of the saucer string*/
#define ROCKET "^"		/* The Rocket String*/
#define TURRET "|"		/* The Turret String*/
#define	TUNIT   20000		/* timeunits in microseconds */
#define MAXSAUCERS 200		/* An Obsurdly large max saucer count*/
#define MAXROCKETS 200		/* The maximum number of in flight rockets */
#define	AIRSPACE 8		/* The number of lines at the top that can have saucers in them*/
#define MAXESCAPED 20		/* Max number of pods that can escape */
#define DEFAULTROCKETS 30	/* Number of starting Rockets*/

struct	saucer{
	char	str[SAUCER_LEN];	/* the message */
	int	row;	/* the row     */
	int col;	/* The column */
	int	delay;  /* delay in time units */
	//int	dir;	/* +1 or -1	*/
	int	index;	/*Location in the collision grid*/
	int 	alive;	/*Indicates if this saucer is alive*/
	int 	die;
};

struct rocket{
	char *str;	/*The rocket String*/
	int row;	/*The row*/
	int col;	/*The column*/
	int delay;	/*The Speed of the rocket*/
	int dir;	/* +1 or -1*/
	int alive;	/*Indicates if this rocket is alive*/
};

struct gridPoint{
	int row;
	int col;
};

pthread_t      	pSaucers[MAXSAUCERS];	/* The saucer threads		*/
pthread_t      	pRockets[MAXROCKETS];	/* The rocket threads		*/
struct saucer 	sProps[MAXSAUCERS];	/* Properties of Saucers	*/
struct rocket 	rProps[MAXROCKETS];	/* Properties of Rockets	*/
pthread_t	spawnSaucer;
int 		turretCol;
int		numEscapted = 0;
int		numRockets = DEFAULTROCKETS;
int killFlag = 0;
FILE * logFile;

/*LOCKS*/
/*-------------------------------------------------------------------*/
/*ALWAYS attempt to access them in the order they appear here to
* avoid race conditions */
/*-------------------------------------------------------------------*/
/*Lock used for locking curses access */
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
/*Lock used for accessing any of the saucer data structures*/
pthread_mutex_t saucers = PTHREAD_MUTEX_INITIALIZER; 
/*Lock used for accessing any of the rocket data structures*/
pthread_mutex_t rockets = PTHREAD_MUTEX_INITIALIZER; 
/*Lock used for incrementing the escaped saucers variable*/
pthread_mutex_t escaped = PTHREAD_MUTEX_INITIALIZER; 
/*Lock used for incrementing/decrementing the users rocket count*/
pthread_mutex_t currentRockets= PTHREAD_MUTEX_INITIALIZER;
/*Lock used for incrementing/decrementing the users rocket count*/
pthread_mutex_t collision= PTHREAD_MUTEX_INITIALIZER;

/**
* Function prototypes
*/
void resetSaucer(struct saucer * saucer);
void resetRocket(struct rocket * rocket);
void	       *animateSaucer();
void	       *animateRocket();
void	       *saucerSpawn();
void drawTurret();
void spawnRocket();
void printInfo();
void setup();
void gameOver();
int checkCollision(int myRow, int myCol);

/**
* Main!
*/
int main(int ac, char *av[])
{
	int	       c;			/* user input			*/
	setup();
	//void drawTurret();
	//pthread_create(&spawnSaucer, NULL, saucerSpawn, NULL);
	/* process user input */
	while(1) {
		c = getch();
		if ( c == 'Q' ) break;
		
		if ( (c == 'a' || c == KEY_LEFT) && (killFlag != 1)){
			if(turretCol > 1){
				turretCol = turretCol - 1;
			}
			drawTurret();
		}
		if ( (c == 'd' || c == KEY_RIGHT) && (killFlag != 1) ){
			if(turretCol < (COLS - 1)){
				turretCol = turretCol + 1;
			}
			drawTurret();
		}
		if( c == ' ' && killFlag != 1){
			if(numRockets > 0){
				spawnRocket();
				pthread_mutex_lock(&currentRockets);
				numRockets--;
				pthread_mutex_unlock(&currentRockets);
				printInfo();
			}
		}
	}
	endwin();
	fclose(logFile);
	return 0;
}
/**
* Handles a game over
*/
void gameOver(){
	int i;
	/* cancel all the threads */
	killFlag = 1;
	pthread_cancel(spawnSaucer);
	for (i=0; i<MAXSAUCERS; i++ )
		pthread_cancel(pSaucers[i]);
	for (i=0; i<MAXROCKETS; i++ )
		pthread_cancel(pRockets[i]);
	
	pthread_mutex_lock(&mx);	/* only one thread	*/
	move( (LINES/2), ((COLS/2) - 5) );	/* can call curses	*/
	addch(' ');			/* at a the same time	*/
	addstr( "GAME OVER" );		/* Since I doubt it is	*/
	addch(' ');			/* reentrant		*/
	move(LINES-1,COLS-1);	/* park cursor		*/
	refresh();			/* and show it		*/
	pthread_mutex_unlock(&mx);	/* done with curses	*/
}
/**
* Game set-up
*/
void setup()
{
	int i;
	
	logFile = fopen("log.txt", "w+");
	
	pthread_create(&spawnSaucer, NULL, saucerSpawn, NULL);

	/*reset all saucers to default (tucked the collision grid in there too)*/
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
	keypad(stdscr, TRUE);
	
	turretCol = COLS / 2;
	
	printInfo();
	mvprintw(LINES-2,turretCol,TURRET);
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
	//saucer->dir = 1;
	saucer->alive = -1;
	saucer->col = 0;
	saucer->index = -1;
	saucer->die = 0;
}
/**
* Sets a rocket struct to default settings
*/
void resetRocket(struct rocket * rocket){
	rocket->str = ROCKET;
	rocket->row = (LINES - 3);
	rocket->col = 0;
	rocket->delay = 3;
	rocket->dir = -1;
	rocket->alive = -1;
}

/**
* Code that runs each saucer thread
*/
void *animateSaucer(void *arg)
{
	struct saucer *info = arg;		/* point to info block	*/
	int	len = strlen(info->str)+2;	/* +2 for padding	*/

	while( 1 )
	{
		usleep(info->delay*TUNIT);
		
		if(info->die == 1){
			pthread_mutex_lock(&saucers);
			info->alive = -1;
			pthread_mutex_unlock(&saucers);
			pthread_exit(NULL);
		}
		
		pthread_mutex_lock(&mx);	/* only one thread	*/
		   move( info->row, info->col );	/* can call curses	*/
		   addch(' ');			/* at a the same time	*/
		   addstr( info->str );		/* Since I doubt it is	*/
		   addch(' ');			/* reentrant		*/
		   move(LINES-1,COLS-1);	/* park cursor		*/
		   refresh();			/* and show it		*/
		pthread_mutex_unlock(&mx);	/* done with curses	*/

		/* move item to next column and check for bouncing	*/
		pthread_mutex_lock(&saucers);
		info->col += 1;
		pthread_mutex_unlock(&saucers);
		
		if(info->col >= COLS){
			pthread_mutex_lock(&saucers);
			info->alive = -1;
			pthread_mutex_unlock(&saucers);
			pthread_mutex_lock(&escaped);
				numEscapted++;
			pthread_mutex_unlock(&escaped);
			printInfo();
			if(numEscapted >= MAXESCAPED)
			{
				gameOver();
			}
			pthread_exit(NULL);
		}
		/*Used to dynamically shrink the print string as we hit the end of the
		screen in order to ensure no wrap around*/
		if(info->col + len >= COLS){
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
				sProps[i].index = i;
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

void spawnRocket(){
int i;
pthread_mutex_lock(&rockets);
		for(i=0; i < MAXROCKETS; i++){
			if(rProps[i].alive == -1){
				resetRocket(&rProps[i]);
				rProps[i].col = turretCol;
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
	
	pthread_mutex_lock(&mx);	/* only one thread	*/
		   move( info->row, info->col );	/* can call curses	*/
		   addstr( info->str );		/* Since I doubt it is	*/
		   move(LINES-1,COLS-1);	/* park cursor		*/
		   refresh();			/* and show it		*/
	pthread_mutex_unlock(&mx);	/* done with curses	*/

	info->row += info->dir;
	
	while( 1 )
	{
		usleep(info->delay*TUNIT);

		pthread_mutex_lock(&mx);	/* only one thread	*/
		   move( info->row + 1, info->col );	/* can call curses	*/
		   addch(' ');			/* at a the same time	*/
		   move( info->row, info->col );	/* can call curses	*/
		   addstr( info->str );		/* Since I doubt it is	*/
		   move(LINES-1,COLS-1);	/* park cursor		*/
		   refresh();			/* and show it		*/
		pthread_mutex_unlock(&mx);	/* done with curses	*/
		
		/*Check for collision, Exit if we find collision*/
		//fprintf(logFile, "Calling collision\n"); 
		if(checkCollision(info->row, info->col) == 1){
			//fprintf(logFile, "Inside collision\n"); 
			//fprintf(logFile, "Call collision"); 
			pthread_mutex_lock(&mx);	/* only one thread	*/
			move( info->row, info->col );	/* can call curses	*/
			addch(' ');			/* at a the same time	*/
			move(LINES-1,COLS-1);	/* park cursor		*/
			refresh();			/* and show it		*/
			pthread_mutex_unlock(&mx);	/* done with curses	*/
			pthread_mutex_lock(&rockets);
			info->alive = -1;
			pthread_mutex_unlock(&rockets);
			pthread_exit(NULL);
		}
	//fprintf(logFile, "Continue\n"); 
		/* move item to next column and check for bouncing	*/
		info->row += info->dir;

		if(info->row < 0){
			pthread_mutex_lock(&mx);	/* only one thread	*/
		   	move( info->row + 1, info->col );	/* can call curses	*/
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

int checkCollision(int myRow, int myCol){
	int i;
	for(i=0; i<MAXSAUCERS;i++){
		if(sProps[i].row == myRow && sProps[i].alive != -1){
			if((myCol >= sProps[i].col) && (myCol <= (sProps[i].col + SAUCER_LEN))){
				pthread_mutex_lock(&saucers);
				sProps[i].die = 1;
				pthread_mutex_unlock(&saucers);
				pthread_mutex_lock(&mx);	/* only one thread	*/
				move( sProps[i].row, sProps[i].col );	/* can call curses	*/
				addstr( "     " );		
				//move(LINES-1,COLS-1);	/* park cursor		*/
				refresh();			/* and show it		*/
				pthread_mutex_unlock(&mx);	/* done with curses	*/
				pthread_mutex_lock(&currentRockets);
					numRockets += 5;
				pthread_mutex_unlock(&currentRockets);
				printInfo();
				return 1;
			}
		}
	}
	fprintf(logFile, "%d %d\n", myRow, myCol); 
	pthread_mutex_unlock(&collision);
	return 0;
}


