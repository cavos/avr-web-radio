/*
 * _23k256.c
 *
 *  Author: Krzysztof Kwiecinski
 */
#include <avr/io.h>
#include <avr/interrupt.h> 
#include <avr/wdt.h>
#include "mcp23k256.h"

volatile	static UINT16	dat = 0;

void	mcp23kInit()
{
	MCP23K_ENABLE();
	
	spi_write(MCP23K_WRSR);	// write status register
	spi_write(MCP23K_WR_SEQUENTIAL); // sequential write mode
	
	MCP23K_DISABLE();
	
	dat = 0;
}

UINT16	mcp23kWrite(UINT16 addr, UINT8 *data, UINT16 len)
{
	if ((addr > MCP23K_SIZE) /*|| (mcp23kFree() == 0)*/)
	{
		LED_ON();
		return 0;
	}
	
	//UINT16 i=0;
	
	//if (mcp23kFree() < len)
	//{
		//len = mcp23kFree();
	//}

	MCP23K_ENABLE();
	
	spi_write(MCP23K_WRITE);
	spi_write(addr>>8);
	spi_write(addr&0xFF);
	wdt_reset();
	//for (i = 0x0000; i < len; i++)
	//{
		//spi_write(*data);
		//LED_ON();
		//data++;
	//}
	while(len--)
	{
		// write data
		SPDR = *data++;
		wait_spi();
	}
	
	
	MCP23K_DISABLE();

	
	dat += len;
	
	return len;
}

UINT16	mcp23kRead(UINT16 addr, UINT8 *data, UINT16 len)
{
	if ((addr > MCP23K_SIZE) /*|| (mcp23kUsed() == 0)*/)
	{
		return 0;
	}
	
	UINT16 l2 = 0x00;
	
	if (len > mcp23kUsed() || mcp23kUsed() == 0x0000)
	{
		len = mcp23kUsed();
	}
	
	MCP23K_ENABLE();
	
	spi_write(MCP23K_READ);
	spi_write(addr>>8);
	spi_write(addr&0xFF);
	//for (i = 0x00; i < len; i++)
	//{
		//LED_ON();
		//*data = spi_read();
		//data++;
	//}
	
	while(len--)
	{
		// read data
		
		SPDR = 0xFF;
		l2++;
		wait_spi();
		*data++ = SPDR;
	}
	
	MCP23K_DISABLE();
	
	dat -= l2;
	
	return l2;
}

UINT16	mcp23kFree()
{
	return (MCP23K_SIZE - dat);
}

UINT16	mcp23kUsed()
{
	return dat;
}