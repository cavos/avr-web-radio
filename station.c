/*
 * station.c
 *
 * Created: 2012-12-23 11:21:26
 *  Author: krzychu
 */ 
#include <avr/pgmspace.h>
#include "station.h"
#include "fifo.h"
#include <string.h>
#include "rtsp.h"
#include "shoutcast.h"
#include "vs1053.h"

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
		shoutcastClose();
		//rtsp_close();
		//vs_stop();
		station_status = STATION_CLOSED;
		
		vsInitialize();
	}
}

UINT8	stationOpen(UINT8 item)
{
	if(item >= ITEM_COUNT){return 0xFF;}
	UINT8 r;
	UINT8 proto;
	
	station_item = item;
	station_status = STATION_OPEN;
	station_timeouts = STATION_RETRIES;
	
	//vsStart();
	
	r = STATION_ERROR;
	stationProto(item, &proto);
	if (proto == PROTO_SHOUTCAST)
	{
		r = shoutcastOpen(item);
	}
	else if(proto == PROTO_RTSP)
	{
		r = rtspOpen(item);
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
	UINT8 dat[32];
	
	switch(station_status)
	{
		case STATION_OPENED:
			station_timeouts = 0;
			
			if(fifoLength() < station_minBuf) // check buffer
			{
				station_status =STATION_BUFFERING;
			}
			else
			{
				while((vsCheckDreq() != 0) && (fifoLength() >= 32))
				{
					fifoPop(dat, 32);
					vsData(dat);
				}
			}
			break;
			
		case STATION_BUFFERING:
		//LED_ON();
			if (fifoLength() > station_playBuf)
			{
				station_status = STATION_OPENED;
				LED_OFF();
			}
			if(station_timeouts >= STATION_TIMEOUT)
			{
				stationClose();
				station_status = STATION_OPEN;
			}
			break;
			
		case STATION_OPEN:
			if (fifoLength() > station_playBuf)
			{
				station_status = STATION_OPENED;
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

void	stationAddr(UINT8 item, ipAddr *ip, UINT16 *port, char *url)
{	
	switch(item)
	{
		case 0:
			strcpy_P(url, PSTR("http://72.233.84.175/")); // DUBSTEP.FM 64K
			ip->b8[3] = 72;
			ip->b8[2] = 233;
			ip->b8[1] = 84;
			ip->b8[0] = 175;
			
			*port = 8030;
			break;
		case 1:
			strcpy_P(url, PSTR("http://217.74.72.11/")); // RMF FM
			ip->b8[3] = 217;
			ip->b8[2] = 74;
			ip->b8[1] = 72;
			ip->b8[0] = 11;
			
			*port = 9000;
			break;
		case 2:
			strcpy_P(url, PSTR("http://192.168.137.1"));
			ip->b8[3] = 192;
			ip->b8[2] = 168;
			ip->b8[1] = 137;
			ip->b8[0] = 1;
			
			*port = 8554;
			break;
		case 3:
			strcpy_P(url, PSTR("/"));
			ip->b8[3] = 192;
			ip->b8[2] = 168;
			ip->b8[1] = 10;
			ip->b8[0] = 100;
			
			*port = 8000;
			break;
	}
	
	
}

void	stationProto(UINT8 item, UINT8 *proto)
{
	switch(item)
	{
		case 0:
			//strcpy_P((char*)proto, PSTR("shoutcast"));
			*proto = PROTO_SHOUTCAST;
			break;
		case 1:
			//strcpy_P((char*)proto, PSTR("shoutcast"));
			*proto = PROTO_SHOUTCAST;
			break;
		case 2:
			//strcpy_P((char*)proto, PSTR("rtsp"));
			*proto = PROTO_RTSP;
			break;
			
		case 3:
			*proto = PROTO_SHOUTCAST;
			break;
	}
}

void	stationName(UINT8 item, UINT8 *n)
{
	switch(item)
	{
		case 0:
			strcpy_P((char*)n, PSTR("DUBSTEP.FM\n"));
			break;
		case 1:
			strcpy_P((char*)n, PSTR("RMF FM\n"));
			break;
		case 2:
			strcpy_P((char*)n, PSTR("RTSP test\n"));
			break;
		case 3:
			strcpy_P((char*)n, PSTR("SHOUTCAST test\n"));
		break;
	}
}