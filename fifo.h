/*
 * fifo.h
 *
 *  Author: Krzysztof Kwiecinski
 *	
 */ 


#ifndef FIFO_H_
#define FIFO_H_
#include "mcp23k256.h"

#define FIFO_SIZE	(MCP23K_SIZE)

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
UINT16	fifoPut( UINT8 *b, UINT16 len );

///<summary>
/// Retrieves data from queue 
/// UINT8 *b - pointer to data
/// UINT16 len - length of data
/// returns amount of data read
///</summary>
UINT16	fifoPop( UINT8 *b, UINT16 len );

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

///<summary>
/// Get total size of fifo queue
/// return size of fifo queue
///</summary>
UINT16	fifoSize();

#endif /* FIFO_H_ */