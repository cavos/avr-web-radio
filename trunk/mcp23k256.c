/*
 * _23k256.c
 *
 *  Author: Krzysztof Kwiecinski
 */
#include <avr/io.h> 
#include "mcp23k256.h"

UINT16	dat = 0;

void	mcp23kInit()
{
	MCP23K_ENABLE();
	
	spi_write(MCP23K_WRSR);	// write status register
	spi_write(MCP23K_WR_SEQUENTIAL); // sequential write mode
	
	MCP23K_DISABLE();
}

UINT16	mcp23kWrite(UINT16 addr, UINT8 *data, UINT16 len)
{
	if (addr > MCP23K_SIZE)
	{
		return 0;
	}
	
	UINT8 i;
	
	if (mcp23kFree() < len)
	{
		len = mcp23kFree();
	}
	
	MCP23K_ENABLE();
	
	spi_write(MCP23K_WRITE);
	spi_write(addr>>8);
	spi_write(addr&0xFF);
	for (i = 0x00; i < len; i++)
	{
		spi_write(*data);
		data++;
	}
	
	MCP23K_DISABLE();
	
	dat += len;
	
	return len;
}

UINT16	mcp23kRead(UINT16 addr, UINT8 *data, UINT16 len)
{
	UINT8 i;
	
	if (len > mcp23kUsed() || mcp23kUsed() == 0x0000)
	{
		len = mcp23kUsed();
	}
	
	MCP23K_ENABLE();
	
	spi_write(MCP23K_READ);
	spi_write(addr>>8);
	spi_write(addr&0xFF);
	for (i = 0x00; i < len; i++)
	{
		*data = spi_read();
		data++;
	}
	
	MCP23K_DISABLE();
	
	dat -= len;
	
	return len;
}

UINT16	mcp23kFree()
{
	return (MCP23K_SIZE - dat);
}

UINT16	mcp23kUsed()
{
	return dat;
}