
/*
 * rtsp.c
 *
 * Created: 2013-01-03 13:24:05
 *  Author: krzychu
 */ 
#include "rtsp.h"
#include "main.h"
#include "eth.h"
#include "station.h"


volatile	UINT8	rtsp_status = RTSP_CLOSED;
UINT16	rtsp_localport = 0;

void	rtspClose()
{
	UINT32 timeout;
	
	if(rtsp_status != RTSP_CLOSED)
	{
		rtsp_status = RTSP_TEARDOWN;
		
		timeout = get_time() + 3;
		
		while(rtsp_status != RTSP_CLOSED)
		{
			ethService();
			
			if (timeout < get_time())
			{
				rtsp_status = RTSP_CLOSED;
				break;
			}
		}
	}
}

UINT16	rtspOpen(UINT8 item)
{
	UINT32	timeout;
	UINT8	index, trying, status;
	ipAddr	ip;
	UINT16	port;
	char	addr[15];
	
	rtsp_status = RTSP_OPEN;
	rtsp_localport = RTSP_CLIENTPORT;
	
	stationAddr(item, &ip, &port, addr);
	index = tcpConnect(ip, port, rtsp_localport, 2048);
	timeout = get_time() + RTSP_TIMEOUT;
	trying = RTSP_TRIES;
	
	for(;;)
	{
		ethService();
		
		status = rtsp_status;
		if ((status == RTSP_CLOSED)		||
			(status == RTSP_OPENED)		||
			(status == RTSP_ERROR))
		{
			break;
		}
		
		if (timeout < get_time())
		{
			timeout = get_time() + RTSP_TIMEOUT;
			if(--trying)
			{
				rtsp_status =RTSP_OPEN;
				index = tcpConnect(ip, port, rtsp_localport, 2048);
			}
			else
			{
				rtsp_status = RTSP_ERROR;
				tcpAbort(index);
				break;
			}
		}
	}
	
	if (rtsp_status == RTSP_OPENED)
	{
		return STATION_OPENED;
	}
	
	return STATION_ERROR;
}