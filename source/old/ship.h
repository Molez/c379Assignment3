#ifndef _SHIP_H_
#define _SHIP_H_

#define	SAUCER "<--->"		/* The saucer String*/
#define SAUCER_LEN 5		/*The length of the saucer string*/
#define MAXSAUCERS 200		/* An Obsurdly large max saucer count*/

struct	saucer{
	char	str[SAUCER_LEN];	/* the message */
	int	row;	/* the row     */
	int	delay;  /* delay in time units */
	int	dir;	/* +1 or -1	*/
	int 	alive;	/*Indicates if this saucer is alive*/
};

void initShips();
void stopAllSaucers();
void resetSaucer(struct saucer * saucer);

#endif