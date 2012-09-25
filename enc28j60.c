#define	F_CPU	12228000UL
#include <avr/io.h>
#include <util/delay.h>
#include "enc28j60.h"
#include "types.h"
#include "main.h"
//#include "enc28j60conf.h"

UINT8 Enc28j60Bank;
UINT16 NextPacketPtr;

UINT8 enc28j60_readOp(UINT8 op, UINT8 address)
{
	UINT8 data;
   
	// assert CS
	ENC28J60_ENABLE();
	
	// issue read command
	SPDR = op | (address & ADDR_MASK);
	wait_spi();
	// read data
	SPDR = 0x00;
	wait_spi();
	// do dummy read if needed
	if(address & 0x80)
	{
		SPDR = 0x00;
		wait_spi();
	}
	data = SPDR;
	
	// release CS
	ENC28J60_DISABLE();
	
	return data;
}

void en28j60_writeOp(UINT8 op, UINT8 address, UINT8 data)
{
	// assert CS
	ENC28J60_ENABLE();

	// issue write command
	SPDR = op | (address & ADDR_MASK);
	wait_spi();
	// write data
	SPDR = data;
	wait_spi();

	// release CS
	ENC28J60_DISABLE();
}

void enc28j60_readBuffer(UINT16 len, UINT8* data)
{
	// assert CS
	ENC28J60_ENABLE();
	
	// issue read command
	SPDR = ENC28J60_READ_BUF_MEM;
	wait_spi();
	while(len--)
	{
		// read data
		SPDR = 0x00;
		wait_spi();
		*data++ = SPDR;
	}	
	// release CS
	ENC28J60_DISABLE();
}

void enc28j60_writeBuffer(UINT16 len, UINT8* data)
{
	// assert CS
	ENC28J60_ENABLE();
	
	// issue write command
	SPDR = ENC28J60_WRITE_BUF_MEM;
	wait_spi();
	while(len--)
	{
		// write data
		SPDR = *data++;
		wait_spi();
	}	
	// release CS
	ENC28J60_DISABLE();
}

void enc28j60_setBank(UINT8 address)
{
	// set the bank (if needed)
	if((address & BANK_MASK) != Enc28j60Bank)
	{
		// set the bank
		en28j60_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		en28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		Enc28j60Bank = (address & BANK_MASK);
	}
}

UINT8 enc28j60_read(UINT8 address)
{
	// set the bank
	enc28j60_setBank(address);
	// do the read
	return enc28j60_readOp(ENC28J60_READ_CTRL_REG, address);
}

void enc28j60_write(UINT8 address, UINT8 data)
{
	// set the bank
	enc28j60_setBank(address);
	// do the write
	en28j60_writeOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

UINT16 enc28j60_phyRead(UINT8 address)
{
	UINT16 data;

	// Set the right address and start the register read operation
	enc28j60_write(MIREGADR, address);
	enc28j60_write(MICMD, MICMD_MIIRD);

	// wait until the PHY read completes
	while(enc28j60_read(MISTAT) & MISTAT_BUSY);

	// quit reading
	enc28j60_write(MICMD, 0x00);
	
	// get data value
	data  = enc28j60_read(MIRDL);
	data |= enc28j60_read(MIRDH);
	// return the data
	return data;
}

void enc28j60_phyWrite(UINT8 address, UINT16 data)
{
	// set the PHY register address
	enc28j60_write(MIREGADR, address);
	
	// write the PHY data
	enc28j60_write(MIWRL, data);	
	enc28j60_write(MIWRH, data>>8);

	// wait until the PHY write completes
	while(enc28j60_read(MISTAT) & MISTAT_BUSY);
}

void enc28j60_init(void)
{
	// perform system reset
	en28j60_writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	// check CLKRDY bit to see if reset is complete
	_delay_us(50);
	while(!(enc28j60_read(ESTAT) & ESTAT_CLKRDY));

	// do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers, must write low byte first
	// set receive buffer start address
	NextPacketPtr = RXSTART_INIT;
	enc28j60_write(ERXSTL, RXSTART_INIT&0xFF);
	enc28j60_write(ERXSTH, RXSTART_INIT>>8);
	// set receive pointer address
	enc28j60_write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60_write(ERXRDPTH, RXSTART_INIT>>8);
	// set receive buffer end
	// ERXND defaults to 0x1FFF (end of ram)
	enc28j60_write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60_write(ERXNDH, RXSTOP_INIT>>8);
	// set transmit buffer start
	// ETXST defaults to 0x0000 (beginning of ram)
	enc28j60_write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60_write(ETXSTH, TXSTART_INIT>>8);

	// do bank 2 stuff
	// enable MAC receive
	enc28j60_write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	// bring MAC out of reset
	enc28j60_write(MACON2, 0x00);
	// enable automatic padding and CRC operations
	en28j60_writeOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
//	enc28j60_write(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	// set inter-frame gap (non-back-to-back)
	enc28j60_write(MAIPGL, 0x12);
	enc28j60_write(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	enc28j60_write(MABBIPG, 0x12);
	// Set the maximum packet size which the controller will accept
	enc28j60_write(MAMXFLL, MAX_FRAMELEN&0xFF);	
	enc28j60_write(MAMXFLH, MAX_FRAMELEN>>8);

	// do bank 3 stuff
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	enc28j60_write(MAADR5, ENC28J60_MAC0);
	enc28j60_write(MAADR4, ENC28J60_MAC1);
	enc28j60_write(MAADR3, ENC28J60_MAC2);
	enc28j60_write(MAADR2, ENC28J60_MAC3);
	enc28j60_write(MAADR1, ENC28J60_MAC4);
	enc28j60_write(MAADR0, ENC28J60_MAC5);

	// no loopback of transmitted frames
	enc28j60_phyWrite(PHCON2, PHCON2_HDLDIS);

	// switch to bank 0
	enc28j60_setBank(ECON1);
	// enable interrutps
	en28j60_writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
	// enable packet reception
	en28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

void enc28j60_sendPacket(unsigned int len1, unsigned char* packet1, unsigned int len2, unsigned char* packet2)
{
	//Errata: Transmit Logic reset
	en28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
	en28j60_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);

	// Set the write pointer to start of transmit buffer area
	enc28j60_write(EWRPTL, TXSTART_INIT&0xff);
	enc28j60_write(EWRPTH, TXSTART_INIT>>8);
	// Set the TXND pointer to correspond to the packet size given
	enc28j60_write(ETXNDL, (TXSTART_INIT+len1+len2));
	enc28j60_write(ETXNDH, (TXSTART_INIT+len1+len2)>>8);

	// write per-packet control byte
	en28j60_writeOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

	// copy the packet into the transmit buffer
	enc28j60_writeBuffer(len1, packet1);
	if(len2>0) enc28j60_writeBuffer(len2, packet2);
	
	// send the contents of the transmit buffer onto the network
	en28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

unsigned int enc28j60_receivePacket(unsigned int maxlen, unsigned char* packet)
{
	UINT16 rxstat;
	UINT16 len;

	// check if a packet has been received and buffered
//	if( !(enc28j60_read(EIR) & EIR_PKTIF) )
	if( !enc28j60_read(EPKTCNT) )
		return 0;

	// Set the read pointer to the start of the received packet
	enc28j60_write(ERDPTL, (NextPacketPtr));
	enc28j60_write(ERDPTH, (NextPacketPtr)>>8);
	// read the next packet pointer
	NextPacketPtr  = enc28j60_readOp(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= enc28j60_readOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	// read the packet length
	len  = enc28j60_readOp(ENC28J60_READ_BUF_MEM, 0);
	len |= enc28j60_readOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	// read the receive status
	rxstat  = enc28j60_readOp(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= enc28j60_readOp(ENC28J60_READ_BUF_MEM, 0)<<8;

	// limit retrieve length
	// (we reduce the MAC-reported length by 4 to remove the CRC)
	//len = MIN(len, maxlen);
	len = len>maxlen?maxlen:len;

	// copy the packet from the receive buffer
	enc28j60_readBuffer(len, packet);

	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	enc28j60_write(ERXRDPTL, (NextPacketPtr));
	enc28j60_write(ERXRDPTH, (NextPacketPtr)>>8);

	// decrement the packet counter indicate we are done with this packet
	en28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

	return len;
}

UINT8	enc28j60_getRevision()
{
	return enc28j60_read(EREVID);
}