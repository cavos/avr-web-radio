/*
 * rtsp.h
 *
 * Created: 2012-12-29 10:22:44
 *  Author: krzychu
 */ 


#ifndef RTSP_H_
#define RTSP_H_

#include "eth.h"
#include "types.h"

#define RTSP_SERVERPORT		(554)
#define RTSP_CLIENTPORT		(1015)
#define RTSP_TIMEOUT		(5)
#define RTSP_TRIES			(3)

#define RTSP_CLOSED			(0)
#define RTSP_TEARDOWN		(1)
#define RTSP_OPENED			(2)
#define RTSP_OPEN			(3)
#define RTSP_PLAY			(4)
#define RTSP_SETUP			(5)
#define RTSP_ERROR			(6)

#define RTSP_OFFSET			(ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE)
#define RTSP_HEADER_SIZE	(4)

typedef struct
{
	UINT8	magic; // Magic number - 0x24
	UINT8	channel;
	UINT16	length;	
}RTSP_HEADER;


#define RTP_OFFSET			(ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE + RTSP_HEADER_SIZE)
#define RTP_HEADER_SIZE		(12)

typedef struct
{
	UINT8	flags;
	UINT8	type;
	UINT16	seq;
	UINT32	time;
	UINT32	ssrc;
}RTP_HEADER;

//////////////////////////////////////////////////////////////////////////
void	rtspClose();
UINT16	rtspOpen(UINT8 item);
void	rtspPutData(UINT8 *data, UINT16 len);
void	rtspTcpApp(UINT8 index, UINT8 *data, UINT16 len);

#endif /* RTSP_H_ */