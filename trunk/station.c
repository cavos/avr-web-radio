/*
 * station.c
 *
 * Created: 2012-12-23 11:21:26
 *  Author: krzychu
 */ 
#include <avr/pgmspace.h>
#include "station.h"
#include "fifo.h"

UINT8	station_item;
UINT8	station_status;
UINT8	station_timeouts;
UINT8	station_try;
UINT16	station_minBuf;
UINT16	station_playBuf;
UINT16	station_hiBuf;


void	stationInit()
{
	UINT32 tmp = fifoFree();
	
	station_item = 0;
	station_status = STATION_CLOSED;
	station_timeouts = 0;
	station_try = STATION_RETRIES;
	
	station_minBuf = (tmp*10)/100; // 10% buffer, revert into buffer mode
	station_playBuf = (tmp*50)/100; // 50% buffer, play mode
	station_hiBuf = (tmp*75)/100; // 75% buffer, 
}

void	stationClose()
{
	if (station_status != STATION_CLOSED)
	{
		//shoutcast_close();
		//rtsp_close();
		//vs_stop();
		station_status = STATION_CLOSED;
	}
}

UINT8	stationOpen(UINT8 item)
{
	if(item >= ITEM_COUNT){return 0xFF;}
	UINT8 r;
	UINT8 proto[10];
	
	station_item = item;
	station_status = STATION_OPEN;
	station_timeouts = STATION_RETRIES;
	
	//vsStart();
	
	r = STATION_ERROR;
	stationProto(item, proto);
	if (strcmp(proto, "shoutcast"))
	{
		//r = shoutcastOpen();
	}
	else if(strcmp(proto, "rtsp"))
	{
		//r = rtspOpen();
	}
	
	if (r == STATION_OPENED)
	{
		station_timeouts = 0;
	}
	
	return r;
}

void	stationService()
{
	//UINT16 len;
	
	switch(station_status)
	{
		case STATION_OPENED:
			station_timeouts = 0;
			
			if(fifoBuffer() < station_minBuf) // check buffer
			{
				station_status =STATION_BUFFERING;
				//vsPause();
			}
			else
			{
				//while (/*vsBuffFree()*/ < 32) // przerzucanie zawartoœci bufora do vs
				{
					
				}
				//vsPlay();
			}
			break;
			
		case STATION_BUFFERING:
			if (fifo_buffer() > station_playBuf)
			{
				station_status = STATION_OPENED;
				//vsPlay();
			}
			if(station_timeouts >= STATION_TIMEOUT)
			{
				stationClose();
				station_status = STATION_OPEN;
			}
			break;
			
		case STATION_OPEN:
			if (fifo_buffer() > station_playBuf)
			{
				station_status = STATION_OPENED;
				//vsPlay();
			}
			if (station_timeouts >= STATION_TIMEOUT)
			{
				stationClose();
				if (station_try)
				{
					station_try--;
					stationOpen(station_item);
				}
			}
			break;
			
		case STATION_CLOSED:
			break;
	}
}

void	stationAddr(UINT8 item, ipAddr *ip, UINT16 *port, UINT8 *url)
{
	char addr[32];
	UINT8 *ptr = &addr[0];
	
	switch(item)
	{
		case 0:
			strcpy_P(addr, PSTR("72.233.84.175:8030")); // DUBSTEP.FM 64K
			break;
		case 1:
			strcpy_P(addr, PSTR("217.74.72.11:9000")); // RMF FM
			break;
		case 2:
			
			break;
	}
}

void	stationProto(UINT8 item, UINT8 *proto)
{
	switch(item)
	{
		case 0:
			strcpy_P(proto, PSTR("shoutcast"));
			break;
		case 1:
			strcpy_P(proto, PSTR("shoutcast"));
			break;
		case 2:
			strcpy_P(proto, PSTR("rtsp"));
			break;
	}
}

void	stationName(UINT8 item, UINT8 *n)
{
	switch(item)
	{
		case 0:
			strcpy_P(n, PSTR("DUBSTEP.FM\n"));
			break;
		case 1:
			strcpy_P(n, PSTR("RMF FM\n"));
			break;
		case 2:
			strcpy_P(n, PSTR("RTSP test\n"));
			break;
	}
}