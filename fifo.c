/*
 * fifo.c
 *
 *  Author: Krzysztof Kwiecinski
 */ 

#define	F_CPU	12228000UL
#include "types.h"
#include "main.h"
#include "fifo.h"
#include <avr/io.h>
#include <util/delay.h>

UINT16	fifo_head;
UINT16	fifo_tail;

void	fifo_init(void)
{
	fifo_tail = 0x0000;
	fifo_head = 0x0000;
	
	BUFFER_ENABLE();
	spi_write(FIFO_WRSR);
	spi_write(MODE_SEQUENTIAL);
	BUFFER_DISABLE();
	
}

void	fifo_put( UINT8* b, UINT16 len )
{
	UINT8 i;
	
	if ( len > fifo_free() )
	{
		return;
	}
	BUFFER_ENABLE();
	spi_write(FIFO_WRITE);
	spi_write(fifo_head>>8);
	spi_write(fifo_head&0xFF);
	for (i = 0x00; i < len; i++)
	{
		spi_write(*b);
		b++;
	}
	
	fifo_head = fifo_head + len;
	fifo_head = fifo_head&(FIFO_SIZE-1);
	
	BUFFER_DISABLE();
}

UINT8	fifo_pop( UINT8* b, UINT8 len )
{
	UINT8 i;
	if ( len > fifo_buffer() || fifo_buffer() == 0x0000)
	{
		len = fifo_buffer();
	}
	
	BUFFER_ENABLE();

	spi_write( FIFO_READ );
	spi_write( fifo_tail>>8);
	spi_write( fifo_tail&0xFF);
	for( i=0x00; i < len; i++)
	{
		*b = spi_read();
		b++;
	}
	BUFFER_DISABLE();
	
	fifo_tail = fifo_tail + len;
	fifo_tail = fifo_tail&(FIFO_SIZE-1);
	
	return len;
}

void	fifo_pop32(UINT8 *b)
{
	UINT8 i;
	
	if (fifo_buffer() < 32)
	{
		*b=0xAA;
		b++;
		*b=0xFB;
		
		return;
	}
	
	spi_write(FIFO_READ);
	spi_write( fifo_tail>>8);
	spi_write( fifo_tail&0xFF);
	for( i=0x00; i < 32; i++)
	{
		*b = spi_read();
		b++;
	}
	BUFFER_DISABLE();
	
	fifo_tail = fifo_tail + 32;
	fifo_tail = fifo_tail&(FIFO_SIZE-1);
	
	return;
}

UINT16	fifo_free()
{
	UINT16 free;
	
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
	
	return free-1;
}

UINT16 fifo_buffer()
{
	UINT16 buffer;
	
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
	
	return buffer;
}

void	fifo_spi_write( UINT8 b )
{
	spi_2x_clr();
	SPDR = b;
	wait_spi();
	
	spi_2x();
}

UINT8	fifo_spi_read( void )
{
	spi_2x_clr();
	UINT8 b;
	
	SPDR = 0xff;
	wait_spi();
	b = SPDR;
	
	spi_2x();
	
	return b;
}