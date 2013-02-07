/*
 * AVR_webradio.c
 *
 *  Author: Krzysztof Kwiecinski
 */ 

#define	F_CPU	12228000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include "types.h"
#include "main.h"
#include "fifo.h"
#include "enc28j60.h"
#include "eth.h"
#include "mcp23k256.h"
#include "shoutcast.h"
#include "station.h"
#include "vs1053.h"


UINT32	get_time(void);

volatile	UINT32	time=0;
volatile	UINT8	run_timeservice;
UINT16	tmp;  
device	settings;

UINT8 dupa[536];

int main(void)
{
	/* ports */
	PORTA	= B0 | B1 | B2 | B3 | B4 | B5 | B6 | B7;
	DDRA	= B0 | B1 | B2 | B3 | B4 | B5 | B6 | B7;
	PORTB	= 0x00;
	PORTB	= B0 | B1 | B2 | B3 | B4 |      B6    ;
	DDRB	= 0x00;
	DDRB	= B0 | B1 |	     B3 | B4 | B5 |      B7;
	PORTC	= B0 | B1 | B2 | B3 | B4 | B5 | B6 | B7;
	DDRC	= B0 | B1 | B2 | B3 | B4 | B5 | B6 | B7;
	PORTD	= B0 | B1 | B2 | B3 | B4 | B5 | B6 | B7;
	DDRD	= 0x00;
	DDRD	= B0 | B1 |							 B7;
	
	/* watchdog */
	wdt_enable(WDTO_4S);
	wdt_reset();
	
	/* ~1 second timer */
	TCNT1	=	0;
	TCCR1A	=	0;
	TCCR1B	=	(1<<WGM12)|(1<<CS12)|(0<<CS11)|(1<<CS10);
	OCR1A	=	(F_CPU/1024)-1;
	TIMSK1	|=	(1<<OCIE1A);
	
	/* SPI */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);
    SPSR |= (1<<SPI2X);
	
	//settings.gateway.b32 = 0xc0a80164; // 192.168.1.100
	//settings.ipaddr.b32 = 0xc0a80173; // 192.168.1.115
	settings.gateway.b8[3] = 192;
	settings.gateway.b8[2] = 168;
	settings.gateway.b8[1] = 10;
	settings.gateway.b8[0] = 100;
	settings.ipaddr.b8[3] = 192;
	settings.ipaddr.b8[2] = 168;
	settings.ipaddr.b8[1] = 10;
	settings.ipaddr.b8[0] = 115;
	settings.mac.b8[0] = ENC28J60_MAC0;
	settings.mac.b8[1] = ENC28J60_MAC1;
	settings.mac.b8[2] = ENC28J60_MAC2;
	settings.mac.b8[3] = ENC28J60_MAC3;
	settings.mac.b8[4] = ENC28J60_MAC4;
	settings.mac.b8[5] = ENC28J60_MAC5;
	settings.netmask.b32 = 0xFFFFFF00;
	
	//VS_XCS_DISABLE();
	
	//LED_ON();
	//LED_OFF();
	
	enc28j60_init();
	
	mcp23kInit();
	fifoInit();

	_delay_ms(600);
	LED_OFF();

	ethInit();
	
	vsInitialize();
	stationInit();
	sei();
	
	
	//fifoPut(dupa, 512);
	//fifoPut(dupa, 512);
	//if(fifoLength() > (fifoSize()/2)){ LED_ON();}
	
	
/************************************************************************/
/* main loop                                                            */
/************************************************************************/
    while(1)
    {
        wdt_reset();
		
		ethService();
		stationService();
		
		//if (fifoLength() < station_minBuf)
		//{
			//LED_ON();
		//}
		//if (fifoLength() > station_playBuf)
		//{
			//LED_OFF();
		//}
		
		if (BUTTON_S4 == 0) // cancel play
		{
			_delay_ms(150); // contact bounce
			
			stationClose();
		}
		else if (BUTTON_S5 == 0)
		{
			_delay_ms(150);
			
			stationOpen(SHOUTCAST_TEST);
		}
		else if (BUTTON_S2 == 0)
		{
			_delay_ms(150);
			//LED_ON();
			fifoPut(dupa, 536);
			//PORTA = fifoLength()&0xFF;
			//PORTC = fifoLength()>>8;
			//LED_OFF();
		}
		else if (BUTTON_S3 == 0)
		{
			_delay_ms(150);
			
			((fifoPop(dupa, 512)) == 0)?LED_ON():LED_OFF();
			//PORTA = fifoLength()&0xFF;
			//PORTC = fifoLength()>>8;
		}
	
		if( run_timeservice )
		{
			ethTimeService();
			run_timeservice = 0x00;
		}		
    }
}

ISR( TIMER1_COMPA_vect )
{
	time = time + 1;
	run_timeservice = 0xFF;
	//LED_TOGGLE();
	
	return;
}

UINT32	get_time(void)
{
	UINT32 i;
	
	cli();
	i = time;
	sei();
	
	return i;
}

int	get_deltaTime(UINT32 *time)
{
	int tmp;
	
	cli();
	tmp = get_time() - *time;
	sei();
	
	return tmp;
}

void	spi_write(UINT8 b)
{
	SPDR = b;
	wait_spi();
}

UINT8	spi_read()
{
	UINT8 b;
	b = 0xFF;
	
	SPDR = b;
	wait_spi();
	b = SPDR;
	
	return b;
}

UINT8	spi_transfer(UINT8 b)
{
	SPDR = b;
	wait_spi();
	
	return SPDR;
}