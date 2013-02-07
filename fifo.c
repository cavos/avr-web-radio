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
#include <avr/interrupt.h>

UINT16	fifo_head;
UINT16	fifo_tail;

void	fifoInit(void)
{
	fifo_tail = 0x0000;
	fifo_head = 0x0000;
}

UINT16	fifoPut( UINT8* b, UINT16 len )
{
	if (fifoFree() < len)
	{
		len = fifoFree();
	}
	mcp23kWrite(fifo_head, b, len);
	
	fifo_head = fifo_head + len;
	fifo_head = fifo_head&(MCP23K_SIZE-1);
	
	return len;
}

UINT16	fifoPop( UINT8* b, UINT16 len )
{
	if (fifoLength() < len)
	{
		return 0;
	}
	
	len = mcp23kRead(fifo_tail, b, len);
	fifo_tail = fifo_tail + len;
	fifo_tail = fifo_tail&(MCP23K_SIZE-1);
	
	return len;
}

UINT16	fifoFree()
{
	UINT16 free;
	
	if ( fifo_head > fifo_tail)
	{
		free = (MCP23K_SIZE - (fifo_head - fifo_tail))-1;
	}
	else if (fifo_head < fifo_tail)
	{
		free = (fifo_tail - fifo_head)-1;
	}
	else
	{
		free = (MCP23K_SIZE - 1);
	}
	
	return free;
}

UINT16 fifoLength()
{
	UINT16 buffer;
	
	if (fifo_head > fifo_tail)
	{
		buffer = fifo_head - fifo_tail;
	}
	else if (fifo_head < fifo_tail)
	{
		buffer = (MCP23K_SIZE - (fifo_tail-fifo_head));
	}
	else
	{
		buffer = 0;
	}
	
	return buffer;
}

UINT16	fifoSize()
{
	return FIFO_SIZE;
}