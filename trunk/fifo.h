/*
 * fifo.h
 *
 *  Author: Krzysztof Kwiecinski
 *	
 */ 


#ifndef FIFO_H_
#define FIFO_H_

/* prototypes */
void	fifoInit( void );	/* initializes fifo queue */
void	fifoPut( UINT8 *b, UINT16 len );	/* writes memory */
UINT8	fifoPop( UINT8 *b, UINT8 len );	/* reads memory, returns read amount of data*/

UINT16	fifoFree(void);	/* returns amount of free memory */
UINT16	fifoLength(void);	/* retruns amount of buffered data */

#endif /* FIFO_H_ */