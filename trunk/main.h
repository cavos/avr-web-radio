#ifndef _MAIN_H
#define _MAIN_H
#include "types.h"
#include <avr/io.h>
/*=================================================================*/

/*/--------------------------- defines */

#define APPNAME		"AVRWebRadio"
#define APPVER		"0.1.0"

#define STATION_PORT	1000

#ifndef F_CPU
#define	F_CPU	12228000UL
#endif


/* ENC28J60 */
#define	ENC28J60_DISABLE()	(PORTB |= B3)
#define ENC28J60_ENABLE()	(PORTB &= ~B3)

/* VS1053 */
#define VS_XCS_ENABLE()		(PORTD &= ~B0)
#define VS_XCS_DISABLE()	(PORTD |= B0)
#define VS_XDCS_ENABLE()	(PORTD &= ~B1)
#define VS_XDCS_DISABLE()	(PORTD |= B1)
#define VS_DREQ()			(PORTD & B2)

/* buffer */
#define MCP23K_ENABLE()		(PORTB &= ~B0)
#define MCP23K_DISABLE()	(PORTB |= B0)

/* buttons */
#define BUTTON_S4			(PIND & B6)
#define BUTTON_S2			(PIND & B5)
#define BUTTON_S3			(PIND & B4)
#define BUTTON_S5			(PIND & B3)

/* LED */
#define LED_ON()			( PORTD &= ~B7 )
#define LED_OFF()			( PORTD |= B7 )
#define LED_TOGGLE()		( PORTD ^= B7 )


#define wait_spi()			while(!(SPSR & (1<<SPIF)))
#define spi_2x()			SPSR |= (1<<SPI2X);
#define spi_2x_clr()		SPSR &= ~(1<<SPI2X);

void	spi_write( UINT8 b);
UINT8	spi_read( );
UINT8	spi_transfer(UINT8 b);
UINT32	get_time(void);
int	get_deltaTime(UINT32 *time);

extern volatile	UINT32 time;

#endif