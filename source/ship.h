#ifndef SHIP_H
#define SHIP_H

/*Global Definitions*/
#define	SAUCER "<--->"		/* The saucer String*/
#define SAUCER_LEN 5		/*The length of the saucer string*/

struct	saucer{
	char	str[SAUCER_LEN];	/* the message */
	int	row;	/* the row     */
	int	delay;  /* delay in time units */
	int	dir;	/* +1 or -1	*/
	int 	alive;	/*Indicates if this saucer is alive*/
};

/**
* Function prototypes
*/
void	       *animateSaucer();
void	       *saucerSpawn();
void resetSaucer(struct saucer * saucer);
#endif