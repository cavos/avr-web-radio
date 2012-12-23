/*
 * mcp23k256.h
 *
 *  Author: Krzysztof Kwiecinski
 */ 


#ifndef mcp23K256_H_
#define mcp23K256_H_

#include "main.h"
#include "types.h"

#define MCP23K_SIZE	0x8000

// instruction set ------------------------------
#define MCP23K_READ		0x03	/* read data form memory array beginning at selected address */
#define MCP23K_WRITE	0x02	/* write data to memory array beginning at selected address*/
#define MCP23K_RDSR		0x05	/* read STATUS register */
#define MCP23K_WRSR		0x01	/* write STATUS register*/

// status register ------------------------------
#define MCP23K_MODE1	0x80
#define MCP23K_MODE0	0x40
#define MCP23K_HOLD		0x01

// write modes ----------------------------------
#define MCP23K_WR_BYTE			0x01
#define MCP23K_WR_PAGE			0x81
#define MCP23K_WR_SEQUENTIAL	0x41

// proto ----------------------------------------
///<summary>
/// Initializes Microchip 23k256 SRAM memory
///</summary>
void	mcp23kInit();

///<summary>
/// Write operation, returns amount of data written to memory
///</summary>
UINT16	mcp23kWrite(UINT16 addr, UINT8 *data, UINT16 len);

///<summary>
/// Read operation, retruns amout of data has been read
///</summary>
UINT16	mcp23kRead(UINT16 addr, UINT8 *data, UINT16 len);

///<summary>
/// Returns amount of free memory 
///</summary>
UINT16	mcp23kFree();

///<summary>
/// Returns amout of allocated memory
///</summary>
UINT16	mcp23kUsed();

#endif /* 23K256_H_ */