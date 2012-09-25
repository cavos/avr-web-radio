/*
 * fifo.h
 *
 *  Author: Krzysztof Kwiecinski
 *	
 * Microchip 25K256
 *  HOLD mode is disabled
 */ 


#ifndef FIFO_H_
#define FIFO_H_

#define	FIFO_SIZE	0x8000

/* instruction set */
#define FIFO_READ	0x03	/* read data form memory array beginning at selected address */
#define FIFO_WRITE	0x02	/* write data to memory array beginning at selected address*/
#define FIFO_RDSR	0x05	/* read STATUS register */
#define FIFO_WRSR	0x01	/* write STATUS register*/

/* status register*/
#define FIFO_MODE1	0x80
#define FIFO_MODE0	0x40
#define FIFO_HOLD	0x01

/* write modes */
#define MODE_BYTE		0x01
#define MODE_PAGE		0x81
#define MODE_SEQUENTIAL	0x41

/* prototypes */
void	fifo_init( void );	/* initializes fifo queue */
void	fifo_put( UINT8 *b, UINT16 len );	/* writes memory */
UINT8	fifo_pop( UINT8 *b, UINT8 len );	/* reads memory, returns read amount of data*/
void	fifo_pop32( UINT8 *b);				/* read 32B of data */

UINT16	fifo_free(void);	/* returns amount of free memory */
UINT16	fifo_buffer(void);	/* retruns amount of buffered data */

void	fifo_spi_write( UINT8 b );
UINT8	fifo_spi_read( void );

#endif /* FIFO_H_ */