/*
 * shoutcast.c
 *
 * Created: 2013-01-03 20:02:52
 *  Author: krzychu
 */ 

#include "shoutcast.h"
#include <stdio.h>
#include "eth.h"
#include "fifo.h"

volatile UINT8	shout_status = SHOUTCAST_CLOSED;
UINT16	shout_localport = 0;
ipAddr	shout_Ip;
UINT16	shout_Port;
char	shout_url[32];


UINT8	shoutcastOpen(UINT8 item)
{
	UINT32	timeout;
	UINT8	index, tries, status;
	
	shout_localport = SHOUTCAST_CLIENTPORT;
	stationAddr(item, &shout_Ip, &shout_Port, NULL);
	
	index = tcpConnect(stationIp,stationPort, shout_localport);
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
			break;
		}
		
		if (get_dealtaTime(&timeout) > 0)
		{
			timeout = get_time() + SHOUTCAST_TIMEOUT;
			
			if (--tries)
			{
				shout_status = SHOUTCAST_OPEN;
				index = tcpConnect(stationIp,stationPort, shout_localport);
			}
			else
			{
				shout_status = SHOUTCAST_TIMEOUTERR;
				tcpAbort(index);
				return SHOUTCAST_TIMEOUTERR;
			}
		}
		
		if (shout_status == SHOUTCAST_OPENED)
		{
			return SHOUTCAST_OPENED;
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
			
			if (get_deltaTime(timeout) > 0)
			{
				shout_status = SHOUTCAST_CLOSED;
				return;
			}
		}
	}
}

void	shoutcastTcpApp(UINT8 index, UINT8 *data, UINT16 len)
{
	UINT16 len;
	UINT8 i;
	char buffer[32], *ptr;
	
	switch(shout_status)
	{
		case SHOUTCAST_OPEN:
			shout_status = SHOUTCAST_HEADER;
			
			len = sprintf((char*)&packetBuffer[TCP_DATA], "GET %s HTTP/1.0\r\n"
														  "Host %i.%i.%i.%i\r\n"
														  "Accept: */*\r\n"
														  "Icy-MetaData: 0\r\n"
														  "Connection: close\r\n"
														  "\r\n",
														  shout_url, shout_Ip.b8[0], shout_Ip[1], shout_Ip[2], shout_Ip[3]);
			tcpSend(index)
		break;
		
		case SHOUTCAST_HEADER:
			shout_status = SHOUTCAST_OPENED;
		break;
		
		case SHOUTCAST_OPENED:
			shoutcastData(data, len);
			
			tcpSend(index, 0); // ACK
		break;
		
		case SHOUTCAST_CLOSE:
			shout_status = SHOUTCAST_CLOSED;
			tcpAbort(index);
			tcpSend(index,0);
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
	while(bufLen) // send data to VS1053
	{
		if(/*vsBufferFree()*/ < 32)
		{
			break;
		}
		if (bufLen < 32)
		{
			fifoPop(d, len);
			//vsBufferPut(d, len);
		}
		else
		{
			fifoPop(d, 32);
			//vsBufferPut(d, 32);
			bufLen -= 32;
		}
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
}