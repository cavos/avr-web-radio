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

volatile UINT8	shout_status = SHOUTCAST_CLOSED;
UINT16	shout_localport = 0;
ipAddr	shout_Ip;
UINT16	shout_Port;
char	shout_url[32];


UINT8	shoutcastOpen(UINT8 item)
{
	UINT32	timeout;
	UINT8	index, status;
	char	tries;
	
	LED_ON();
	
	shout_localport = SHOUTCAST_CLIENTPORT;
	stationAddr(item, &shout_Ip, &shout_Port, shout_url);
	shout_status = SHOUTCAST_OPEN;
	
	index = tcpConnect(shout_Ip,shout_Port, shout_localport);
	timeout = get_time()+SHOUTCAST_TIMEOUT;
	tries = SHOUTCAST_TRY;
	
	//udpDbgSend(PSTR("Shout->Open"), 11);
	
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
		
		if (get_deltaTime(&timeout) > 0)
		{
			timeout = get_time() + SHOUTCAST_TIMEOUT;
			
			if (tries > 0)
			{
				tries = tries - 1;LED_TOGGLE();
				shout_status = SHOUTCAST_OPEN;
				index = tcpConnect(shout_Ip,shout_Port, shout_localport);
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
			//udpDbgSend(PSTR("Shout->Opened"),13);
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
	//UINT8 i;
	//char buffer[32];
	
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
			tcpSend(index, txlen);
			LED_OFF();
			//udpDbgSend(PSTR("Shout->Sent open msg"), 20);
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
		if(/*vsBufferFree()*/ bufLen< 32)
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
}