/*
 * fifo.h
 *
 *  Author: Krzysztof Kwiecinski
 *	
 */ 


///<TODO>
/// add interface to make it separate form mcp23k256
///</TODO>
#ifndef FIFO_H_
#define FIFO_H_

/* prototypes */
///<summary>
/// Initializes fifo queue, initializes pointers, can be used to reset fifo queue
///</summary>
void	fifoInit( void );

///<summary>
/// Puts data into queue
/// UINT8 *b - pointer to data
/// UINT16 len - length of data
///</summary>
void	fifoPut( UINT8 *b, UINT16 len );

///<summary>
/// Retrieves data from queue 
/// UINT8 *b - pointer to data
/// UINT16 len - length of data
/// returns amount of data read
///</summary>
UINT8	fifoPop( UINT8 *b, UINT8 len );

///<summary>
/// Calculates amount of free memory in queue
/// returns amount of free memory
///</summary>
UINT16	fifoFree(void);

///<summary>
///	Calculates amount of data buffered within queue
/// returns length of queue (amount of data bufered)
///</summary>
UINT16	fifoLength(void);

#endif /* FIFO_H_ */