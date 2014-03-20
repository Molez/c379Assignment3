#ifndef GAME_H
#define GAME_H

/*Global Definitions*/
#define TURRET "|"		/* The Turret String*/
#define	TUNIT   20000		/* timeunits in microseconds */
#define MAXSAUCERS 200		/* An Obsurdly large max saucer count*/
#define MAXROCKETS 200		/* The maximum number of in flight rockets */
#define	AIRSPACE 8		/* The number of lines at the top that can have saucers in them*/
#define MAXESCAPED 20		/* Max number of pods that can escape */
#define DEFAULTROCKETS 30	/* Number of starting Rockets*/

/**
* Function prototypes
*/
void setup();
void printInfo();
void drawTurret();
void getTurretCol();
void decrementTurret();
void incrementTurret()
#endif