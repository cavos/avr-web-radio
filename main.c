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
#include "types.h"
#include "main.h"
#include "fifo.h"
#include "enc28j60.h"
#include "eth.h"
//#include "tux_enc28j60.h"
//#include "tux_avr_compat.h"
//#include "ethernet.h"


UINT32	get_time(void);

volatile	UINT32	time=0;
volatile	UINT8	run_timeservice;
//static		UINT8	my_mac[6] = {0x00,0x55,0x58,0x10,0x50,0x11};
//static		UINT8	my_mac[6] = {'x','R','A','D','I','O'};
UINT8 a1[] = {0x48, 0x4F, 0x53, 0x54, 0x50, 0x43, 0x30, 0x52, 0x41, 0x44, 0x49, 0x4F, 0x08, 0x00, 0x45,
			  0x00, 0x00, 0x2E, 0x00, 0x00, 0x40, 0x00, 0x40, 0x11, 0x0A, 0xB9, 0x98, 0x00, 0x00, 0x04,
			  0x98, 0x00, 0x00, 0x02, 0x10, 0x00, 0x20, 0x00, 0x00, 0x1A, 0xCF, 0xB3, 0xD0, 0x00, 0x44,
			  0x55, 0x50, 0x41, 0x20, 0x49, 0x4E, 0x4F, 0x20, 0x44, 0x55, 0x50, 0x41, 0x20, 0x54, 0x45,
			  0x53, 0x54, 0x20, 0x50, 0x41, 0x43, 0x4B, 0x45, 0x54, 0x00};
			  
device	settings;

int main(void)
{
	//UINT8 msg84[]= {0x54, 0x04, 0xA6, 0x07, 0x0B, 0x13, 0x00, 0x52, 0x41, 0x44, 0x49, 0x4F, 0x08, 0x00, 0x45, 0x00, 0x00, 0x2E,
	//			  0x00, 0x00, 0x40, 0x00, 0x40, 0x11, 0x79, 0x08, 0xC0, 0xA8, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//			  0x00, 0x00, 0x00, 0x1A, 0x3E, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//UINT8 a2[1000];
	//UINT16 ba = 0xC0F0;
	
	
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
	
	settings.gateway.b32 = 0xc0a80164; // 192.168.1.100
	settings.ipaddr.b32 = 0xc0a80173; // 192.168.1.115
	settings.mac.b8[0] = ENC28J60_MAC0;
	settings.mac.b8[1] = ENC28J60_MAC1;
	settings.mac.b8[2] = ENC28J60_MAC2;
	settings.mac.b8[3] = ENC28J60_MAC3;
	settings.mac.b8[4] = ENC28J60_MAC4;
	settings.mac.b8[5] = ENC28J60_MAC5;
	settings.netmask.b32 = 0xFFFFFF00;
	
	enc28j60_init();
	
	fifo_init();
	
	//enc28j60_sendPacket(67, a1, 0, 0);
	//enc28j60_sendPacket(64, a1);
	PORTA = enc28j60_getRevision();
	
	ethInit();
	LED_OFF();
	sei();
	
/************************************************************************/
/* main loop                                                            */
/************************************************************************/
    while(1)
    {
        wdt_reset();
		
		ethGetData();
				
		if( run_timeservice )
		{
			//eth_timeService();
			run_timeservice = 0x00;
			//LED_TOGGLE();
		}		
		else if( !BUTTON_1 )
		{
			enc28j60_sendPacket(67, a1, 0, 0);
			LED_ON();
			_delay_ms(100);
			LED_OFF();
		}			
    }
}

ISR( TIMER1_COMPA_vect )
{
	time = time + 1;
	run_timeservice = 0xFF;
	//enc28j60_sendPacket(67, a1, 0, 0);
	
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