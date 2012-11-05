/*
 * tcp_udp.c
 *
 * Created: 2012-10-15 14:41:38
 *  Author: krzychu
 */ 
#define	F_CPU	12228000UL

#include <util/delay.h>
#include <avr/io.h>
#include "main.h"
#include "types.h"
#include "enc28j60.h"
//#include "checksum.h"
#include "eth.h"
#include "fifo.h"

TCP_HEADER	*tcp = (TCP_HEADER*)&packetBuffer[TCP_OFFSET];
TCPtable	tcpTable[MAX_TCP_ENTRY];
TCPtable	*tcpConn;

void	tcpClose(UINT8 index)
{
	tcpTable[index].status = TCP_S_CLOSE;
}

void	tcpAbort(UINT8 index)
{
	tcpTable[index].status = TCP_S_ABORT;
}

UINT8	tcpConnect(ipAddr targetIp, UINT16 dstPort, UINT16 srcPort)
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
	
	tcpSend(i, 0);
	
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
		if( HTONS(tcp->dstPort) == tcpTable[i].localPort && HTONS(tcp->srcPort) == tcpTable[i].port)
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
		
		tcpSend(index,0);
		tcpTable[index].status = TCP_S_CLOSED;
	}
}

void	tcpSOpen(UINT8 index) // syn & ack
{
	if ( (tcp->flags&TCP_FLAG_FIN) || (tcp->flags&TCP_FLAG_RST))
	{
		tcpTable[index].status = TCP_S_CLOSED;
	} 
	else if ( (tcp->flags&TCP_FLAG_SYN) && (tcp->flags&TCP_FLAG_ACK))
	{
		UINT16 len;
		
		tcpTable[index].acknum = HTONS32(tcp->seqNum) + 1;
		tcpTable[index].seqnum = HTONS32(tcp->ackNum);
		tcpTable[index].flags = TCP_FLAG_ACK;
		tcpTable[index].status = TCP_S_OPENED;
		
		//switch(HTONS(tcp->dstPort))
		//{
			//case STATION_PORT:
				//tcpSend(index, len);
			//break;
		//}
		len = tcpTable[index].appCall();
		tcpSend(index,len);
	}
	else
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
		tcpTable[index].acknum = HTONS32(tcp->seqNum) + 1;
		tcpTable[index].seqnum = HTONS32(tcp->ackNum);
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
		
		UINT8 len = HTONS(ip->length) - (IP_HEADER_SIZE +(tcp->length>>2));
		tcpTable[index].acknum = HTONS32(tcp->seqNum) + len;
		tcpTable[index].flags = TCP_FLAG_ACK;
		
		len = tcpTable[index].appCall();
		tcpSend(index,len);
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
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
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
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
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
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
			tcpTable[i].mac.b8[0] = eth->srcMac.b8[0];
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

UINT16	tcpChecksum(UINT8 *data, UINT16 len, ipAddr dstIp)
{
	UINT32 sum = 0;
	
	// pseudo header
	sum = sum + settings.ipaddr.b16[0];
	sum = sum + settings.ipaddr.b16[1];
	sum = sum + dstIp.b16[0];
	sum = sum + dstIp.b16[1];
	sum = sum + HTONS(len);
	sum = sum + HTONS(IP_PR_TCP);
	
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
	
	return ~sum;
}

void	tcpMakeHeader(UINT8 index, UINT16 len)
{
	ip->length = HTONS(IP_HEADER_SIZE+TCP_HEADER_SIZE + len);
	ip->protocol = IP_PR_TCP;
	ipMakeHeader(tcpTable[index].ip);
	
	tcp->srcPort = HTONS(tcpTable[index].localPort);
	tcp->dstPort = HTONS(tcpTable[index].port);
	tcp->seqNum = HTONS32(tcpTable[index].seqnum);
	tcp->ackNum = HTONS32(tcpTable[index].acknum);
	tcp->length = TCP_HEADER_SIZE<<2;
	tcp->flags = tcpTable[index].flags;
	
	UINT16 f;
	f= fifo_free();
	
	if (f > 4096)
	{
		tcp->window = HTONS(2048);
	}
	else if (f > 2048)
	{
		tcp->window = HTONS(1024);
	}
	else if (f > 512)
	{
		tcp->window = HTONS(512);
	}
	else
	{
		tcp->window = HTONS(256);
	}
	
	tcp->checksum = 0;
	tcp->urgentPtr = 0;
	
	tcp->checksum = tcpChecksum((UINT8*)tcp, TCP_HEADER_SIZE+len, tcpTable[index].ip);
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