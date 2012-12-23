#ifndef _MAIN_H
#define _MAIN_H
#include "types.h"
/*=================================================================*/

/*/--------------------------- defines */

#define APPNAME		"AVRWebRadio"
#define APPVER		"0.1.0"

#define STATION_PORT	1000

/* ENC28J60 */
#define	ENC28J60_DISABLE()	PORTB |= B3
#define ENC28J60_ENABLE()	PORTB &= ~B3

/* VS1053 */
//#define	VS_DREQ_CHECK()		( PIND & B3 ) /* 1 - can accept data */
//#define VS_xCS_DISABLE()	PORTB |= B0
//#define VS_xCS_ENABLE()		PORTB &= ~B0
//#define	VS_xDCS_ENABLE()	PORTB |= B1
//#define VS_xDCS_DISABLE()	PORTB |= ~B1

/* EEPROM */
#define MCP23K_ENABLE()		PORTB &= ~B0
#define MCP23K_DISABLE()	PORTB |= B0

/* buttons */
#define BUTTON_1			(PIND & B6)

/* LED */
#define LED_ON()			( PORTD &= ~B7 )
#define LED_OFF()			( PORTD |= B7 )
#define LED_TOGGLE()		( PORTD ^= B7 )


#define wait_spi()			while(!(SPSR & (1<<SPIF)))
#define spi_2x()			SPSR &= (1<<SPI2X);
#define spi_2x_clr()		SPSR &= ~(1<<SPI2X);

void	spi_write( UINT8 b);
UINT8	spi_read( );
UINT8	spi_transfer(UINT8 b);

extern volatile	UINT32 time;

#endif