/*
 * tcp_udp.c
 *
 * Created: 2012-10-15 14:41:38
 *  Author: krzychu
 */ 
#define	F_CPU	12228000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "main.h"
#include "types.h"
#include "enc28j60.h"
#include "eth.h"
#include "fifo.h"
#include "shoutcast.h"

#ifndef NULL
#define NULL 0
#endif


TCP_HEADER	*tcp = (TCP_HEADER*)&packetBuffer[TCP_OFFSET];
TCPtable	tcpTable[MAX_TCP_ENTRY];
TCPtable	*tcpConn;
UDP_HEADER  *udp = (UDP_HEADER *)&packetBuffer[UDP_OFFSET];
UDPtable	udpTable[UDP_MAX_ENTRIES+1];
UINT8		udpDoChecksum = UDP_CHECKSUM_ON;

void	tcpClose(UINT8 index)
{
	//udpDbgSend(PSTR("TCP->Close"), 10);
	tcpTable[index].status = TCP_S_CLOSE;
}

void	tcpAbort(UINT8 index)
{
	//udpDbgSend(PSTR("TCP->Abort"), 10);
	tcpTable[index].status = TCP_S_ABORT;
}

UINT8	tcpConnect(ipAddr targetIp, UINT16 dstPort, UINT16 srcPort, UINT16 window)
{
	UINT8	i;
	
	for (i=0; i < MAX_TCP_ENTRY; i++)	// find free tcpTable entry
	{
		if( tcpTable[i].status == TCP_S_CLOSED )
		{
			break;
		}
	}
	
	if( i == MAX_TCP_ENTRY)	
	{
		return MAX_TCP_ENTRY;
	}
	
	UINT8 j = arpRequest(targetIp);
	
	tcpTable[i].mac.b8[0] = arpTable[j].mac.b8[0];
	tcpTable[i].mac.b8[1] = arpTable[j].mac.b8[1];
	tcpTable[i].mac.b8[2] = arpTable[j].mac.b8[2];
	tcpTable[i].mac.b8[3] = arpTable[j].mac.b8[3];
	tcpTable[i].mac.b8[4] = arpTable[j].mac.b8[4];
	tcpTable[i].mac.b8[5] = arpTable[j].mac.b8[5];
	tcpTable[i].ip.b32 = targetIp.b32;
	tcpTable[i].port = dstPort;
	tcpTable[i].localPort = srcPort;
	tcpTable[i].acknum = 1UL;
	tcpTable[i].seqnum = 15UL;
	tcpTable[i].flags = TCP_FLAG_SYN;
	tcpTable[i].status = TCP_S_OPEN;
	tcpTable[i].time = 0;
	tcpTable[i].error = 0;
	tcpTable[i].window = window;
	
	tcpSend(i, 0);
	//udpDbgSend(PSTR("TCP->Conn"), 9);
	
	return j;
}

void	tcpSend(UINT8 index, UINT16 len)
{
	tcpMakeHeader(index, len);
	
	enc28j60_sendPacket(ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE + len, packetBuffer, 0,0);
	
	tcpTable[index].seqnum += len;
}

void	tcpService()
{
	UINT8 i;
	//UINT16 len = 0;
	
	for (i = 0; i < MAX_TCP_ENTRY; i++)
	{
		if( (HTONS(tcp->dstPort) == tcpTable[i].localPort) &&
		    (HTONS(tcp->srcPort) == tcpTable[i].port))
		{
			break;
		}
	}
	
	if ( i < MAX_TCP_ENTRY ) // tcp connection in table
	{
		tcpTable[i].time = 0;
		
		switch( tcpTable[i].status)
		{	
			case TCP_S_CLOSE:
				tcpSClose(i);
			break;
			
			case TCP_S_OPEN:
				tcpSOpen(i);
			break;
			
			case TCP_S_OPENED:
				tcpSOpened(i);
			break;
			
			case TCP_S_ABORT:
				tcpSAbort(i);
			break;
			
			case TCP_S_FINISH:
				tcpSFinish(i);
			break;
		}
	} 
	else // establish a new connection
	{
		tcpListen(HTONS(tcp->dstPort));
	}
}

void	tcpSClose(UINT8 index) // send FIN
{
	tcpTable[index].acknum = HTONS32(tcp->seqNum) + HTONS(ip->length) - (IP_HEADER_SIZE + (tcp->length>>2));
	tcpTable[index].flags = TCP_FLAG_FIN | TCP_FLAG_ACK;
	tcpTable[index].status = TCP_S_FINISH;
	tcpSend(index, 0);
}

void	tcpSFinish(UINT8 index) // waitig for ACK|FIN, send ACK
{
	if( (tcp->flags&TCP_FLAG_FIN) && ( tcp->flags&TCP_FLAG_ACK))
	{
		tcpTable[index].seqnum = HTONS32(tcp->ackNum);
		tcpTable[index].acknum = HTONS32(tcp->seqNum) + 1;
		tcpTable[index].flags = TCP_FLAG_ACK;
		tcpTable[index].status = TCP_S_CLOSED;
		tcpSend(index,0);
	}
}

void	tcpSOpen(UINT8 index) // syn & ack
{
	if ( (tcp->flags&TCP_FLAG_FIN) || (tcp->flags&TCP_FLAG_RST))
	{
		tcpTable[index].status = TCP_S_CLOSED;
		//udpDbgSend(PSTR("TCP->FIN|RST->CLOSE"),19);
	} 
	else if ( (tcp->flags&TCP_FLAG_SYN) && (tcp->flags&TCP_FLAG_ACK))
	{
		UINT16 len = HTONS(ip->length)-(IP_HEADER_SIZE+(tcp->length>>2));
		
		tcpTable[index].acknum = HTONS32(tcp->seqNum) + 1;
		tcpTable[index].seqnum = HTONS32(tcp->ackNum);
		tcpTable[index].flags = TCP_FLAG_ACK;
		tcpTable[index].status = TCP_S_OPENED;
		
		switch(HTONS(tcp->srcPort))
		{
			case SHOUTCAST_SERVERPORT:
			//LED_OFF();
				shoutcastTcpApp(index, &packetBuffer[TCP_DATA], len-TCP_HEADER_SIZE);
				//tcpSend(index, 0);
			break;
		}
		//len = tcpTable[index].appCall(&packetBuffer[TCP_DATA], 0);
		tcpSend(index,len);
		//udpDbgSend(PSTR("TCP->Opened"),11);
	}
	else  // abort
	{
		tcpTable[index].acknum = HTONS32(tcp->seqNum) + HTONS(ip->length) - (IP_HEADER_SIZE+(tcp->length>>2));
		tcpTable[index].flags = TCP_FLAG_RST;
		tcpTable[index].status = TCP_S_CLOSED;
		tcpSend(index,0);
	}
}

void	tcpSOpened(UINT8 index) //
{
	if (tcp->flags&TCP_FLAG_RST) // abort conn
	{
		tcpTable[index].status = TCP_S_CLOSED;
	} 
	else if(tcp->flags&TCP_FLAG_FIN)
	{
		tcpTable[index].seqnum = HTONS32(tcp->ackNum);
		tcpTable[index].acknum = HTONS32(tcp->seqNum) + 1;
		tcpTable[index].flags = TCP_FLAG_FIN | TCP_FLAG_ACK;
		tcpTable[index].status = TCP_S_FINISH;
		tcpSend(index,0);
	}
	else
	{
		if(HTONS32(tcp->seqNum) != tcpTable[index].acknum) // frame lost
		{
			tcpSend(index,0); // transmit last packet
			return;
		}
		
		UINT16 len = HTONS(ip->length) - (IP_HEADER_SIZE +(tcp->length>>2));
		tcpTable[index].acknum = HTONS32(tcp->seqNum) + len;
		tcpTable[index].flags = TCP_FLAG_ACK;
		
		switch(HTONS(tcp->srcPort))
		{
			case SHOUTCAST_SERVERPORT: // station
				shoutcastTcpApp(index, &packetBuffer[TCP_DATA], len);
				len = 0;
				tcpSend(index,len);
			break;
		}
		
		//if (tcpTable[index].appCall == NULL)
		//{
			//return;
		//}
		//len = tcpTable[index].appCall(&packetBuffer[TCP_DATA], len - TCP_HEADER_SIZE);
		//tcpSend(index,len);
	}
}

void	tcpSAbort(UINT8 index)
{
	tcpTable[index].acknum = HTONS32(tcp->seqNum) + (HTONS(ip->length)) - (IP_HEADER_SIZE + (tcp->length>>2));
	tcpTable[index].flags = TCP_FLAG_RST;
	tcpTable[index].status = TCP_S_CLOSED;
	
	tcpSend(index, 0);
}

UINT8	tcpListen(UINT16 port)
{
	UINT8 i;
	
	for (i=0; i < MAX_TCP_ENTRY; i++) // look for free table index
	{
		if(tcpTable[i].status == TCP_S_CLOSED)
		{
			break;
		}
	}
	
	if(i < MAX_TCP_ENTRY)
	{
		if (tcp->flags == TCP_FLAG_SYN) // passive open
		{
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[1] = eth->srcMac.b8[1];
			tcpTable[i].mac.b8[2] = eth->srcMac.b8[2];
			tcpTable[i].mac.b8[3] = eth->srcMac.b8[3];
			tcpTable[i].mac.b8[4] = eth->srcMac.b8[4];
			tcpTable[i].mac.b8[5] = eth->srcMac.b8[5];
			tcpTable[i].ip.b32 = ip->sourceIP.b32;
			tcpTable[i].port = HTONS(tcp->srcPort);
			tcpTable[i].localPort = HTONS(tcp->dstPort);
			tcpTable[i].acknum = HTONS32(tcp->seqNum) + 1;
			tcpTable[i].seqnum = 15UL;
			tcpTable[i].flags = TCP_FLAG_SYN|TCP_FLAG_ACK;
			tcpTable[i].status = TCP_S_OPENED;
			tcpTable[i].time = 0;
			tcpTable[i].error = 0;
			
			tcpSend(i,0);
			tcpTable[i].seqnum++;
			
			return i;
		} 
		else if(tcp->flags&TCP_FLAG_FIN)
		{
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[1] = eth->srcMac.b8[1];
			tcpTable[i].mac.b8[2] = eth->srcMac.b8[2];
			tcpTable[i].mac.b8[3] = eth->srcMac.b8[3];
			tcpTable[i].mac.b8[4] = eth->srcMac.b8[4];
			tcpTable[i].mac.b8[5] = eth->srcMac.b8[5];
			tcpTable[i].ip.b32 = ip->sourceIP.b32;
			tcpTable[i].port = HTONS(tcp->srcPort);
			tcpTable[i].localPort = HTONS(tcp->dstPort);
			tcpTable[i].acknum = HTONS32(tcp->seqNum) + 1;
			tcpTable[i].seqnum = 15UL;
			tcpTable[i].flags = TCP_FLAG_SYN|TCP_FLAG_ACK;
			tcpTable[i].status = TCP_S_FINISH;
			tcpTable[i].time = 0;
			tcpTable[i].error = 0;
			
			tcpSend(i,0);
			return MAX_TCP_ENTRY;
		}
		else
		{
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[1] = eth->srcMac.b8[1];
			tcpTable[i].mac.b8[2] = eth->srcMac.b8[2];
			tcpTable[i].mac.b8[3] = eth->srcMac.b8[3];
			tcpTable[i].mac.b8[4] = eth->srcMac.b8[4];
			tcpTable[i].mac.b8[5] = eth->srcMac.b8[5];
			tcpTable[i].ip.b32 = ip->sourceIP.b32;
			tcpTable[i].port = HTONS(tcp->srcPort);
			tcpTable[i].localPort = HTONS(tcp->dstPort);
			tcpTable[i].acknum = HTONS32(tcp->seqNum) + 1;
			tcpTable[i].seqnum = 15UL;
			tcpTable[i].flags = TCP_FLAG_SYN|TCP_FLAG_ACK;
			tcpTable[i].status = TCP_S_CLOSED;
			tcpTable[i].time = 0;
			tcpTable[i].error = 0;
			
			tcpSend(i,0);
			
			return MAX_TCP_ENTRY;
		}
	}
	
	return MAX_TCP_ENTRY;
}

UINT16	tcpChecksum(UINT8 index, UINT16 datalength)
{
	UINT32 sum = 0;
	UINT8 *data = &packetBuffer[TCP_OFFSET];
	UINT16 len = HTONS(ip->length) - IP_HEADER_SIZE;
	//LED_OFF();

	sum = sum + HTONS(settings.ipaddr.b16[0]);
	sum = sum + HTONS(settings.ipaddr.b16[1]);
	sum = sum + HTONS(tcpTable[index].ip.b16[0]);
	sum = sum + HTONS(tcpTable[index].ip.b16[1]);
	sum = sum + HTONS(len);
	sum = sum + HTONS(IP_PR_TCP);
	
	for (; len > 1; len -= 2)
	{
		sum += *((UINT16*)data);
		if (sum & 0x80000000)
		{
			sum = (sum&0xFFFF)+(sum>>16);
			//LED_ON();
		}			
		data += 2; // move pointer
	}
	
	if(len) // left-over byte
	{
		sum += + *(UINT8*)data;
	}
	
	while(sum>>16)
	{
		sum = (sum&0xFFFF)+(sum>>16);
	}
	
	return ~sum;
	
	//UINT32 sum = 0;
	//
	//// pseudo header
	//sum = sum + HTONS(settings.ipaddr.b16[0]);
	//sum = sum + HTONS(settings.ipaddr.b16[1]);
	//sum = sum + tcpTable[index].ip.b16[0];
	//sum = sum + tcpTable[index].ip.b16[1];
	//sum = sum + HTONS(datalength+TCP_HEADER_SIZE);
	//sum = sum + HTONS(IP_PR_TCP);
	//
	//sum += checksum(&packetBuffer[TCP_OFFSET], datalength+TCP_HEADER_SIZE);
	//
	//while(sum>>16)
	//{
		//sum = (sum&0xFFFF)+(sum>>16);
	//}
	//
	//return sum;
}

UINT16 udpChecksum(UINT8 index)
{
	UINT32 sum = 0;
	
	if (udpDoChecksum == UDP_CHECKSUM_OFF)
	{
		return 0;
	}
	
	// pseudo header
	sum = sum + settings.ipaddr.b16[0];
	sum = sum + settings.ipaddr.b16[1];
	sum = sum + udpTable[index].ip.b16[0];
	sum = sum + udpTable[index].ip.b16[1];
	sum = sum + HTONS(udp->lenght);
	sum = sum + HTONS(IP_PR_UDP);
	
	sum += checksum(&packetBuffer[UDP_OFFSET], HTONS(udp->lenght));
	
	return ~sum;
}

UINT16	checksum(UINT8 *data, UINT16 len)
{
	UINT32 sum = 0;
	
	//// pseudo header
	//sum = sum + settings.ipaddr.b16[0];
	//sum = sum + settings.ipaddr.b16[1];
	//sum = sum + dstIp.b16[0];
	//sum = sum + dstIp.b16[1];
	//sum = sum + HTONS(len);
	//sum = sum + HTONS(IP_PR_TCP);
	
	for (; len > 1; len -= 2)
	{
		sum = sum + *((UINT16*)data);
		data += 2; // move pointer
	}
	
	if(len) // left-over byte
	{
		sum = sum + *(UINT8*)data;
	}
	
	while(sum>>16)
	{
		sum = (sum&0xFFFF)+(sum>>16);
	}
	
	return sum;
}

void	tcpMakeHeader(UINT8 index, UINT16 len)
{
	ip->length = HTONS(IP_HEADER_SIZE+TCP_HEADER_SIZE + len);
	ip->protocol = IP_PR_TCP;
	ipMakeHeader(tcpTable[index].ip);
	
	tcp->srcPort = HTONS(tcpTable[index].localPort);
	tcp->dstPort = HTONS(tcpTable[index].port);
	tcp->seqNum = HTONS32(tcpTable[index].seqnum);
	if(tcpTable[index].flags & TCP_FLAG_ACK)
	{
		tcp->ackNum = HTONS32(tcpTable[index].acknum);
	}
	else
	{
		tcp->ackNum = 0UL;
	}
	tcp->length = TCP_HEADER_SIZE<<2;
	tcp->flags = tcpTable[index].flags;
	
	//UINT16 f;
	//f= fifoFree();
	//
	//if (f > 4096)
	//{
		//tcp->window = HTONS(2048);
	//}
	//else if (f > 2048)
	//{
		//tcp->window = HTONS(1024);
	//}
	//else if (f > 512)
	//{
		//tcp->window = HTONS(512);
	//}
	//else
	//{
		//tcp->window = HTONS(256);
	//}
	
	tcp->window = tcpTable[index].window;
	tcp->checksum = 0;
	tcp->urgentPtr = 0;
	
	//tcp->checksum = tcpChecksum((UINT8*)tcp, TCP_HEADER_SIZE+len, tcpTable[index].ip);
	tcp->checksum = tcpChecksum(index, len);
}

void	tcpTimeService()
{
	UINT8 i;
	
	for (i = 0; i < MAX_TCP_ENTRY; i++)
	{
		if(tcpTable[i].status == TCP_S_CLOSED)
		{
			continue;
		}
		else if (++tcpTable[i].time > TCP_TIMEOUT)
		{
			if (++tcpTable[i].error > TCP_MAXERROR)
			{
				tcpTable[i].flags = TCP_FLAG_RST;
				tcpTable[i].status = TCP_S_CLOSED;
				tcpSend(i,0);
			}
			else
			{
				tcpTable[i].time = TCP_TIMEOUT;
				tcpSend(i, 0);
			}
		}
	}
}

void	udpDbgSend(const char *data, UINT16 len)
{
	for (UINT16 i = 0; i < len; i++)
	{
		packetBuffer[UDP_DATA+i] = *data++;
	}
	
	UINT8 tmp = udpDoChecksum;
	udpDoChecksum = UDP_CHECKSUM_OFF;
	
	udpMakeHeader(UDP_DEBUG, len);
	
	udpDoChecksum = tmp;
	
	enc28j60_sendPacket((UDP_OFFSET+len), packetBuffer, 0,0);
}

void	udpMakeHeader(UINT8 index, UINT16 len)	
{
	udp->dstPort = HTONS(udpTable[index].dstPort);
	udp->srcPort = HTONS(udpTable[index].localPort);
	udp->lenght = HTONS(len);
	
	ip->protocol = IP_PR_UDP;
	ip->length = HTONS(IP_HEADER_SIZE+UDP_HEADER_SIZE + len);
	ipMakeHeader(settings.gateway);
	
	udp->checksum = udpChecksum(index);
}