#ifndef _GAME_H_
#define _GAME_H_

#define	TUNIT   20000		/* timeunits in microseconds */
#define	AIRSPACE 8		/* The number of lines at the top that can have saucers in them*/
#define TURRET "|"		/* The Turret String*/

void setupGame();
void stopGame();
pthread_mutex_t getCursesLock();
void printInfo();
int getNumEscaped();
int getTurretPosition();
void incrementTurret();
void decrementTurret();
void drawTurret();
#endif