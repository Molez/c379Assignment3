#include	<stdio.h>
#include	<curses.h>
#include	<ncurses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
/******************************************************************************/
/*These are defines that likely should not be touched*/
#define	SAUCER "<--->"		/* The saucer String*/
#define SAUCER_LEN 5		/*The length of the saucer string*/
#define ROCKET "^"			/* The Rocket String*/
#define REWARD "R"			/* The Reward String*/
#define TURRET "|"			/* The 1p Turret String*/
#define TURRET2P "!"		/* The 2p Turret String*/
#define	TUNIT   20000		/* timeunits in microseconds */
/******************************************************************************/
/*These can be used to tune the game if it gets laggy or something*/
#define MAXSAUCERS 100		/* An Obsurdly large max saucer count*/
#define MAXROCKETS 100		/* The maximum number of in flight rockets */
#define MAXREWARDS 100		/* The maximum number of in flight rewards */
#define DRAWDELAY 1			/*The refresh delay*/
#define REFEREEDELAY 2		/*The delay for how often the game referee
				checks the game state*/
/******************************************************************************/
/*These are game settings that can be adjusted to change the game play*/
#define	AIRSPACE 8			/* The number of lines at the top that 
					can have saucers in them*/
#define MAXESCAPED 20		/* Max number of pods that can escape */
#define DEFAULTROCKETS 30	/* Number of starting Rockets*/
#define REWARDDELAY  8  	/* How fast rewards move down the screen */
#define ROCKETDELAYSLOW  3  	/* The slow speed of rockets*/
#define ROCKETDELAYFAST  1  	/* The fast speed of rockets*/
#define	SPEEDUPCHANCE	 5	/* This is used to set the % chance we get a 
				speedupfrom a reward. Chance for speedup is 
				1/SPEEDUPCHANCE
				*/
#define	REWARDCHANCE	3	/* This is used to set the % chance we get a 
				rewardfrom a kill. Chance for reward is 
				1/REWARDCHANCE*/
/******************************************************************************/
/*True/false defines do not touch these!*/
#define	True			1
#define	FALSE			0
/******************************************************************************/
/**
*	Stores information for one saucer
*/
struct	saucer{
	char	str[SAUCER_LEN];	/* the message */
	int	row;	/* the row     */
	int col;	/* The column */
	int	delay;  /* delay in time units */
	int 	alive;	/*Indicates if this saucer is alive*/
	int 	die;
};
/**
*	Stores information for one rocket
*/
struct rocket{
	char *str;	/*The rocket String*/
	int row;	/*The row*/
	int col;	/*The column*/
	int delay;	/*The Speed of the rocket*/
	int dir;	/* +1 or -1*/
	int alive;	/*Indicates if this rocket is alive*/
};
/**
*	Stores information for one reward
*/
struct reward{
	char *str;	/*The reward String*/
	int row;	/*The row*/
	int col;	/*The column*/
	int delay;	/*The Speed of the rocket*/
	int dir;	/* +1 or -1*/
	int alive;	/*Indicates if this reward is alive*/
};

pthread_t      	pSaucers[MAXSAUCERS];	/* The saucer threads	*/
pthread_t      	pRockets[MAXROCKETS];	/* The rocket threads	*/
pthread_t      	pRewards[MAXROCKETS];	/* The rocket threads	*/
struct saucer 	sProps[MAXSAUCERS];	/* Properties of Saucers	*/
struct rocket 	rProps[MAXROCKETS];	/* Properties of Rockets	*/
struct rocket 	rewardProps[MAXROCKETS];	/* Properties of Rewards*/
pthread_t		spawnSaucer;
pthread_t		drawThread;
pthread_t		refereeThread;
int				rocketDelay;
int 			turretCol;
int 			turretCol2p = -1;
int				numEscapted = 0;
int				numRockets = DEFAULTROCKETS;
int				points = 0;
int 			killFlag = 0;	/*used to determine if we need to kill 
					user input*/
int 			initFlag =0;	/*Used to signify that threads have been 
					initialised*/
int 			secondPlayer = 0; /*Flag for second player*/

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
/*Lock used for accessing any of the rewards data structures*/
pthread_mutex_t reward = PTHREAD_MUTEX_INITIALIZER; 
/*Lock used for incrementing the escaped saucers variable*/
pthread_mutex_t escaped = PTHREAD_MUTEX_INITIALIZER; 
/*Lock used for incrementing/decrementing the users rocket count*/
pthread_mutex_t currentRockets= PTHREAD_MUTEX_INITIALIZER;
/*Lock used for incrementing/decrementing the users point count*/
pthread_mutex_t currentPoints= PTHREAD_MUTEX_INITIALIZER;

/**
* Function prototypes
*/
void 			resetSaucer(struct saucer * saucer);
void 			resetRocket(struct rocket * rocket);
void 			resetReward(struct rocket * rocket);
void	       		*animateSaucer(void *arg);
void	       		*animateRocket(void *arg);
void 			*animateReward(void *arg);
void	       		*saucerSpawn();
void 			drawTurret();
void 			spawnRocket();
void 			spawnReward(int myCol, int myRow);
void 			printInfo();
void 			setup();
void 			gameOver();
void 			gameStart();
int 			checkCollision(int myRow, int myCol);
int 			checkTurretCollision(int myRow, int myCol);
void 			*drawScreen();
void 			*gameReferee();

/**
* Main!
*/
int main(int argc, char *argv[])
{
	int	       c;			/* user input*/
	gameStart();
	
	if( argc == 2 && strcmp(argv[1], "2") == 0)
		secondPlayer = 1; /*Flag for 2 player*/
	/* process user input */
	while(1) {
		c = getch();
		if ( c == 'Q' ) break;
		
		if ( c == 'S' ) {
			killFlag = 0;
			setup();
		}
		/*1p right*/
		if ( c == 'a' && (killFlag != 1)){
			if(turretCol > 1){
				turretCol = turretCol - 1;
			}
		}
		/*2p left*/
		if ( c == KEY_LEFT && (killFlag != 1)){
			if(turretCol2p > 1){
				turretCol2p = turretCol2p - 1;
			}
		}
		/*1p right*/
		if ( c == 'd' && (killFlag != 1) ){
			if(turretCol < (COLS - 1)){
				turretCol = turretCol + 1;
			}
		}
		/*2p right*/
		if ( c == KEY_RIGHT && (killFlag != 1) ){
			if(turretCol2p < (COLS - 1)){
				turretCol2p = turretCol2p + 1;
			}
		}
		/*1p shoot*/
		if( (c == ' ' || c == 'w') && killFlag != 1){
			if(numRockets > 0){
				spawnRocket(1);
				pthread_mutex_lock(&currentRockets);
				numRockets--;
				pthread_mutex_unlock(&currentRockets);
				printInfo();
			}
		}
		/*2p shoot*/
		if( c == KEY_UP && killFlag != 1){
			if(numRockets > 0){
				spawnRocket(2);
				pthread_mutex_lock(&currentRockets);
				numRockets--;
				pthread_mutex_unlock(&currentRockets);
				printInfo();
			}
		}	
	}
	endwin();
	pthread_cancel(refereeThread);/*Kill the referee thread upon leaving*/
	return 0;
}

/**
* Handles the game start
*/
void gameStart(){
	killFlag = TRUE;
	
	/* set up curses */
	initscr();
	crmode();
	noecho();
	clear();
	keypad(stdscr, TRUE);
	
	printInfo(); /*Print the updated bottom info*/
	
	move( (LINES / 2)-1, (COLS / 2 - 6));	
	addstr( "************" );		
	move( LINES / 2, (COLS / 2 - 6));	
	addstr( "*GAME START*" );		
	move( (LINES / 2)+1, (COLS / 2 - 6));	
	addstr( "************" );		
	move( (LINES / 2)+2, (COLS / 2 - 6));	
	addstr( "'S' TO START" );		
	move(LINES-1,COLS-1);	
	refresh();		
}

/**
* Handles a game over
*/
void gameOver(){
	int i;
	killFlag = 1;
	printInfo(); /*Print the updated bottom info*/
	if(initFlag == 1){
		pthread_cancel(spawnSaucer);
		pthread_cancel(drawThread);
	}
	
	move( (LINES / 2)-1, (COLS / 2 - 6));	
	addstr( "***********" );		
	move( LINES / 2, (COLS / 2 - 6));	
	addstr( "*GAME OVER*" );		
	move( (LINES / 2)+1, (COLS / 2 - 6));	
	addstr( "***********" );		
	move( (LINES / 2)+2, (COLS / 2 - 6));	
	addstr( "'Q' TO QUIT" );		
	move(LINES-1,COLS-1);	
	refresh();
	
	/*Cancel all the threads if we have already set them*/
	if(initFlag == TRUE){
		for (i=0; i<MAXSAUCERS; i++ )
			pthread_cancel(pSaucers[i]);
		for (i=0; i<MAXROCKETS; i++ )
			pthread_cancel(pRockets[i]);
		for (i=0; i<MAXREWARDS; i++ )
			pthread_cancel(pRewards[i]);
	}
}
/**
* Game set-up
*/
void setup()
{
	int i;
	
	pthread_create(&spawnSaucer, NULL, saucerSpawn, NULL);
	pthread_create(&drawThread, NULL, drawScreen, NULL);
	pthread_create(&refereeThread, NULL, gameReferee, NULL);
	
	rocketDelay = ROCKETDELAYSLOW;

	/*reset all saucers to default*/
	for(i=0; i < MAXSAUCERS; i++){
		resetSaucer(&sProps[i]);
	}
	/*reset all rockets to default*/
	for(i=0; i < MAXROCKETS; i++){
		resetRocket(&rProps[i]);
	}
	
	/*reset all rockets rewards default*/
	for(i=0; i < MAXREWARDS; i++){
		resetReward(&rewardProps[i]);
	}
	
	
	/*Position the turrets at their start locations */
	if(secondPlayer == TRUE){
		turretCol = COLS / 4;
		turretCol2p = (COLS / 4) * 3;
	}else{
		turretCol = COLS / 2;
	}
	
	printInfo();
	mvprintw(LINES-2,turretCol,TURRET);
	initFlag = TRUE;
}
/**
* Sets a saucer struct to default settings
*/
void resetSaucer(struct saucer * saucer){
	strncpy(saucer->str, SAUCER, strlen(SAUCER));
	srand(time(NULL));
	saucer->row = (rand()%AIRSPACE);
	srand(time(NULL));
	saucer->delay = 2+(rand()%14);
	saucer->alive = -1;
	saucer->col = 0;
	saucer->die = 0;
}
/**
* Sets a rocket struct to default settings
*/
void resetRocket(struct rocket * rocket){
	rocket->str = ROCKET;
	rocket->row = (LINES - 3);
	rocket->col = 0;
	rocket->delay = rocketDelay;
	rocket->dir = -1;
	rocket->alive = -1;
}

/**
* Sets a reward struct to default settings
*/
void resetReward(struct rocket * rocket){
	rocket->str = REWARD;
	rocket->row = 0;
	rocket->col = 0;
	rocket->delay = REWARDDELAY;
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

	while( TRUE )
	{
		usleep(info->delay*TUNIT);
		
		/*Check if the thread has been flagged to die*/
		if(info->die == TRUE){
			pthread_mutex_lock(&saucers);
			info->alive = -1;
			pthread_mutex_unlock(&saucers);
			pthread_exit(NULL);
		}

		pthread_mutex_lock(&saucers);
		info->col += 1;
		pthread_mutex_unlock(&saucers);
		/*Check for saucer exiting the screen*/
		if(info->col >= COLS){
			pthread_mutex_lock(&saucers);
			info->alive = -1;
			pthread_mutex_unlock(&saucers);
			pthread_mutex_lock(&escaped);
				numEscapted++;
			pthread_mutex_unlock(&escaped);
			if(numEscapted >= MAXESCAPED)
			{
				gameOver();
			}
			pthread_exit(NULL);
		}
		/*Used to dynamically shrink the print string as we 
		hit the end of the screen in order to ensure no wrap around*/
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
				pthread_create(&pSaucers[i], NULL, 
				animateSaucer, &sProps[i]);
				sProps[i].alive = TRUE;
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
	/*Draw 1st turret*/
	move( LINES-2, turretCol);	
	addstr(TURRET);		
	move(LINES-1,COLS-1);	
	refresh();
	/*Draw second turret*/
	if(secondPlayer == TRUE){
		move( LINES-2, turretCol2p);
		addstr(TURRET2P);		
		move(LINES-1,COLS-1);	
		refresh();
	}
	pthread_mutex_unlock(&mx);
}
/**
*	Creates and spawns a new rocket thread
*/
void spawnRocket(int player){
int i;
pthread_mutex_lock(&rockets);
	/*Find a free rocket struct*/
	for(i=0; i < MAXROCKETS; i++){
		if(rProps[i].alive == -1){
			resetRocket(&rProps[i]);
			if(player ==1){
				rProps[i].col = turretCol;
			}else{
				rProps[i].col = turretCol2p;
			}
			pthread_create(&pRockets[i], NULL, 
			animateRocket, &rProps[i]);
			rProps[i].alive = TRUE;
			break;
		}
	}
pthread_mutex_unlock(&rockets);
}

/**
*	Creates and spawns a new rocket thread
*/
void spawnReward(int myCol, int myRow){
int i;
pthread_mutex_lock(&reward);
	/*Find a free reward Struct*/
	for(i=0; i < MAXREWARDS; i++){
		if(rewardProps[i].alive == -1){
			resetReward(&rewardProps[i]);
			rewardProps[i].col = myCol;
			rewardProps[i].row = myRow;	
			pthread_create(&pRewards[i], NULL, 
			animateReward, &rewardProps[i]);	
			rewardProps[i].alive = TRUE;
			break;
		}
	}
pthread_mutex_unlock(&reward);
}

/**
* Code that runs each rocket on a thread
*/
void *animateRocket(void *arg)
{
	struct rocket *info = arg;		/* point to info block	*/
	
	while( TRUE )
	{
		usleep(info->delay*TUNIT);
		
		/*Check for collision, Exit if we find collision*/
		if(checkCollision(info->row, info->col) == 1){
			pthread_mutex_lock(&rockets);
			info->alive = -1;
			pthread_mutex_unlock(&rockets);
			srand(time(NULL));
			/*Spawn a reward....Maybe*/
			if((rand()%REWARDCHANCE) == 0){
				spawnReward(info->col, info->row);
			}
			pthread_exit(NULL);
		}
		/* move item to next column and check for bouncing	*/
		pthread_mutex_lock(&rockets);
		info->row += info->dir;
		pthread_mutex_unlock(&rockets);

		if(info->row < 0){
			pthread_mutex_lock(&rockets);
			info->alive = -1;
			pthread_mutex_unlock(&rockets);
			pthread_exit(NULL);
		}
	}
}

/**
* Code that runs each reward on a thread. Rewards check for collision and if
* they collide with a turret they will decide what kind fo reward to give.
*/
void *animateReward(void *arg)
{
	struct rocket *info = arg;		/* point to info block	*/
	
	while( TRUE )
	{
		usleep(info->delay*TUNIT);
		
		/*Check for collision, Exit if we find collision*/
		if(checkTurretCollision(info->row, info->col) == 1){
		
			srand(time(NULL));
			/*Chance to get a speed power-up!*/
			if((rand()%SPEEDUPCHANCE) == 0){
				rocketDelay = ROCKETDELAYFAST;
			}else{
				rocketDelay = ROCKETDELAYFAST;
				pthread_mutex_lock(&currentRockets);
				numRockets += 5;
				pthread_mutex_unlock(&currentRockets);
				/*Get some extra points!*/
				pthread_mutex_lock(&currentPoints);
					points += 2;
				pthread_mutex_unlock(&currentPoints);
			}

			printInfo();
			pthread_mutex_lock(&reward);
			info->alive = -1;
			pthread_mutex_unlock(&reward);
			pthread_exit(NULL);
		}
		/* move item to next column and check for bouncing	*/
		pthread_mutex_lock(&reward);
		info->row += info->dir;
		pthread_mutex_unlock(&reward);

		if(info->row > LINES -1){
			pthread_mutex_lock(&reward);
			info->alive = -1;
			pthread_mutex_unlock(&reward);
			pthread_exit(NULL);
		}
	}
}
/**
* Prints the game info at the bottom of the screen
*/
void printInfo(){
	char temp[1024];
	snprintf(temp, 1024, "Quit: 'Q' | "
	"ESCAPED: %d | ROCKETS: %03d | POINTS: %04d ", numEscapted, numRockets, 
	points);
	pthread_mutex_lock(&mx);	/* only one thread	*/
	move( LINES -1, 0 );		/* can call curses	*/
	addstr( temp );		
	move(LINES-1,COLS-1);		/* park cursor		*/
	refresh();			/* and show it		*/
	pthread_mutex_unlock(&mx);	/* done with curses	*/
}

/**
*	Checks if a collision exists between a rocket and a saucer
*/
int checkCollision(int myRow, int myCol){
	int i;
	for(i=0; i<MAXSAUCERS;i++){
		if(sProps[i].row == myRow && sProps[i].alive != -1){
			if((myCol >= sProps[i].col) && (myCol <= (sProps[i].col+ 
			SAUCER_LEN))){
				pthread_mutex_lock(&saucers);
				sProps[i].die = 1;
				pthread_mutex_unlock(&saucers);
				pthread_mutex_lock(&currentRockets);
					numRockets += 3;
				pthread_mutex_unlock(&currentRockets);
				pthread_mutex_lock(&currentPoints);
					points += 1;
				pthread_mutex_unlock(&currentPoints);
				printInfo();
				return 1;
			}
		}
	}
	return 0;
}

/**
*	Checks if a collision exists between a reward and a turret
*/
int checkTurretCollision(int myRow, int myCol){
	if(myRow == (LINES - 2)){
		if(myCol == turretCol || myCol == turretCol2p){
			return TRUE;
		}
	}
	return FALSE;
}

/**
* Code that draws the game screen
*/
void *drawScreen()
{
	int i;
	while(TRUE){
		usleep(DRAWDELAY*TUNIT);
		pthread_mutex_lock(&mx);
		clear();
		/*Draw saucers*/
		for(i=0; i<MAXSAUCERS;i++){
			if(sProps[i].alive != -1 && sProps[i].die != 1){
				move( sProps[i].row, sProps[i].col );
				addstr( sProps[i].str );		
			}
		}
		/*Draw Rockets*/
		for(i=0; i<MAXROCKETS;i++){
			if(rProps[i].alive != -1){
				move( rProps[i].row, rProps[i].col );	
				addstr( rProps[i].str );
			}
		}
		
		/*Draw Rewards*/
		for(i=0; i<MAXREWARDS;i++){
			if(rewardProps[i].alive != -1){	
				move( rewardProps[i].row, rewardProps[i].col );	
				addstr( rewardProps[i].str );
			}
		}
		
		move(LINES-1,COLS-1);	/* park cursor		*/
		refresh();	
		pthread_mutex_unlock(&mx);
		/*These draw functions draw on their own*/
		printInfo(); /*Print the bottom info*/
		drawTurret(); /*Draw the turret*/
	}
}

/**
* Thread to referee the game. This essentially looks for the instant
* Where the user has no rewards on screen plus 0 rockets and the last one leaves 
the screen. The game over caused by too many escapes can be managed by saucers
*/
void *gameReferee(){
	int gameOverFlag;
	int i;
	while(TRUE){
		usleep(REFEREEDELAY*TUNIT);
		gameOverFlag = FALSE;
		if(numRockets <= 0){
			gameOverFlag = TRUE;
			/*Check for alive rockets*/
			for(i=0; i < MAXROCKETS; i++){
				if(rProps[i].alive == TRUE){
					gameOverFlag = FALSE;
					break;
				}
			}
			/*Check for alive rewards*/
			for(i=0; i < MAXREWARDS; i++){
				if(rewardProps[i].alive == TRUE){
					gameOverFlag = FALSE;
					break;
				}
			}
		}
		if(gameOverFlag == TRUE){
			gameOver();
		}
	}
}


