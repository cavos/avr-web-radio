/*
 * fifo.c
 *
 *  Author: Krzysztof Kwiecinski
 */ 

#define	F_CPU	12228000UL
#include "types.h"
#include "main.h"
#include "fifo.h"
#include "mcp23k256.h"
#include <avr/io.h>

UINT16	fifo_head;
UINT16	fifo_tail;

void	fifoInit(void)
{
	fifo_tail = 0x0000;
	fifo_head = 0x0000;
}

void	fifoPut( UINT8* b, UINT16 len )
{
	len = mcp23kWrite(fifo_head, b, len);
	
	fifo_head = fifo_head + len;
	fifo_head = fifo_head&(MCP23K_SIZE-1);
}

UINT8	fifoPop( UINT8* b, UINT8 len )
{
	len = mcp23kRead(fifo_tail, b, len);
	
	fifo_tail = fifo_tail + len;
	fifo_tail = fifo_tail&(MCP23K_SIZE-1);
	
	return len;
}

UINT16	fifoFree()
{
	/*UINT16 free;
	
	if ( fifo_head > fifo_tail)
	{
		free = FIFO_SIZE - (fifo_head - fifo_tail);
	}
	else if (fifo_head < fifo_tail)
	{
		free = fifo_tail - fifo_head;
	}
	else
	{
		free = FIFO_SIZE;
	}
	
	return free-1;*/
	return mcp23kFree();
}

UINT16 fifoLength()
{
	/*UINT16 buffer;
	
	if (fifo_head > fifo_tail)
	{
		buffer = fifo_head - fifo_tail;
	}
	else if (fifo_head < fifo_tail)
	{
		buffer = FIFO_SIZE - (fifo_tail-fifo_head);
	}
	else
	{
		buffer = 0;
	}
	
	return buffer;*/
	return mcp23kUsed();
}