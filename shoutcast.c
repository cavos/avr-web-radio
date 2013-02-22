/*
 * shoutcast.c
 *
 * Created: 2013-01-03 20:02:52
 *  Author: krzychu
 */ 

#include "shoutcast.h"
#include <stdio.h>
#include <avr/pgmspace.h>
#include "eth.h"
#include "fifo.h"
#include "main.h"
#include "vs1053.h"
#include <avr/wdt.h>

volatile UINT8	shout_status = SHOUTCAST_CLOSED;
UINT16	shout_localport = 0;
ipAddr	shout_Ip;
UINT16	shout_Port;
char	shout_url[32];
UINT8	shout_tcpIndex = 0;
UINT16	shout_playBuf = 0x4000;
UINT16	shout_minBuf = 0x0080;
UINT8	shout_playing = 0;

UINT8	shoutcastOpen(UINT8 item)
{
	UINT32	timeout;
	UINT8	index, status;
	char	tries;
	
	shout_localport = SHOUTCAST_CLIENTPORT;
	stationAddr(item, &shout_Ip, &shout_Port, shout_url);
	shout_status = SHOUTCAST_OPEN;
	
	index = tcpConnect(shout_Ip,shout_Port, shout_localport, HTONS(2048));
	timeout = get_time()+SHOUTCAST_TIMEOUT;
	tries = SHOUTCAST_TRY;
	
	while(1)
	{
		ethService();
		
		status = shout_status;
		
		if ((status == SHOUTCAST_CLOSED)	||
			(status == SHOUTCAST_OPENED)	||
			(status == SHOUTCAST_ERROR))
		{
			shout_tcpIndex = index;
			shout_status = status;
			break;
		}
		
		if (get_deltaTime(&timeout) > 0)
		{
			timeout = get_time() + SHOUTCAST_TIMEOUT;
			
			if (tries > 0)
			{
				tries = tries - 1;
				shout_status = SHOUTCAST_OPEN;
				index = tcpConnect(shout_Ip,shout_Port, shout_localport, HTONS(2048));
			}
			else
			{
				shout_status = SHOUTCAST_TIMEOUTERR;
				tcpAbort(index);
				return SHOUTCAST_TIMEOUTERR;
			}
		}
		
		if (shout_status == SHOUTCAST_OPEN)
		{
			return STATION_OPEN;//SHOUTCAST_OPENED;
		}
	}
	
	return SHOUTCAST_ERROR;
}

void	shoutcastClose()
{
	UINT32	timeout;
	
	if (shout_status != SHOUTCAST_CLOSED)
	{
		shout_status = SHOUTCAST_CLOSE;
		
		timeout = get_time() + SHOUTCAST_TIMEOUT;
		
		while(shout_status != SHOUTCAST_CLOSED)
		{
			ethService();
			
			if (get_deltaTime(&timeout) > 0)
			{
				shout_status = SHOUTCAST_CLOSED;
				return;
			}
		}
	}
}

void	shoutcastTcpApp(UINT8 index, UINT8 *data, UINT16 len)
{
	UINT16 txlen;
	
	switch(shout_status)
	{
		case SHOUTCAST_OPEN:
			shout_status = SHOUTCAST_HEADER;
			
			txlen = sprintf((char*)&packetBuffer[TCP_DATA], "GET %s HTTP/1.0\r\n"
														  "Host: %i.%i.%i.%i\r\n"
														  "Connection: Close\r\n"
														  "User-Agent: Webradio\r\n"
														  "Accept: */*\r\n"
														  "Icy-MetaData:0\r\n"														  
														  "\r\n",
														  shout_url, shout_Ip.b8[3], shout_Ip.b8[2], shout_Ip.b8[1], shout_Ip.b8[0]);
			tcpTable[index].flags |= TCP_FLAG_PSH;
			tcpTable[index].window = HTONS(2048);
			
			shout_tcpIndex = index;
			tcpSend(index, txlen);
		break;
		
		case SHOUTCAST_HEADER:
			
			shout_status = SHOUTCAST_OPENED;
		break;
		
		case SHOUTCAST_OPENED:
			//shoutcastData(data, len);
			
			shoutcastBuffer(data, len);
		break;
		
		case SHOUTCAST_CLOSE:
			shout_status = SHOUTCAST_CLOSED;
			tcpAbort(index);
		break;
		
		case SHOUTCAST_CLOSED:
		break;
	}
}

void	shoutcastData(UINT8 *data, UINT16 len)
{
	UINT16	bufLen;
	UINT8	d[32];
	
	bufLen = fifoLength();
	while(len) // send data to VS1053
	{
		bufLen = fifoLength();
		if((bufLen > 32) && (vsCheckDreq() != 0))
		{
			fifoPop(d, 32);
			vsData(d);
			bufLen -= 32;
		}
		
		bufLen = fifoFree(); // send received data to buffer
		
		if (bufLen < len)
		{
			
			fifoPut(data, bufLen);
			data += bufLen;
			len -=bufLen;
		}
		else
		{
			
			fifoPut(data, len);
			break;
		}
		
		//stationService();
	}
	
	bufLen = fifoFree();
	tcpTable[shout_tcpIndex].window = fifoFree()+2;
	//if (bufLen > 4096)
	//{
		//tcpTable[shout_tcpIndex].window = HTONS(2048);
	//}
	//else if (bufLen > 2048)
	//{
		//tcpTable[shout_tcpIndex].window = HTONS(1024);
	//}
	//else if (bufLen > 512)
	//{
		//tcpTable[shout_tcpIndex].window = HTONS(512);
	//}
	//else
	//{
		//tcpTable[shout_tcpIndex].window = HTONS(256);
	//}
}

void	shoutcastBuffer(UINT8 *data, UINT16 len)
{
	UINT16	bufLen = 384; // send received data to buffer
	//UINT16 tmp = 0x00;
	
	while(len)
	{		
		bufLen =  fifoPut(data, len);
		len = len - bufLen;
		stationService();	
	}
	
	bufLen = fifoFree();
	
	if(bufLen > 8192)
	{
		tcpTable[shout_tcpIndex].window = HTONS(4096);
	}
	else if (bufLen > 2048)
	{
		tcpTable[shout_tcpIndex].window = HTONS(1024);
	}
	else if (bufLen > 1024)
	{
		tcpTable[shout_tcpIndex].window = HTONS(512);
	}
	else
	{
		tcpTable[shout_tcpIndex].window = HTONS(256);
	}
}

void	shoutcastPlay()
{
	UINT8 data[32];
	
	while(vsCheckDreq() != 0 && fifoLength() >= 32)
	{
		fifoPop(data, 32);
		vsData(data);
	}
}