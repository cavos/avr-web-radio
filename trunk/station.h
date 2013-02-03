/*
 * station.h
 *
 * Created: 2012-12-22 18:07:35
 *  Author: krzychu
 */ 


#ifndef STATION_H_
#define STATION_H_

#include "types.h"
#include "eth.h"

// defs ---------------------------------------------
#define STATION_TIMEOUT		15
#define STATION_RETRIES		3

#define STATION_CLOSED		0
#define STATION_OPENED		1
#define	STATION_BUFFERING	2
#define STATION_OPEN		3
#define STATION_ERROR		4
#define STATION_ERRTIMEOUT	5

#define ITEM_COUNT	4

#define	PROTO_SHOUTCAST		(22)
#define PROTO_RTSP			(45)

extern	UINT8	station_item;
extern	UINT8	station_status;
extern	UINT8	station_timeouts;
extern	UINT8	station_try;
extern	UINT16	station_minBuf;
extern	UINT16	station_playBuf;
extern	UINT16	station_hiBuf;

// proto -------------------------------------------
extern	void	stationInit();
extern	void	stationClose();
extern	UINT8	stationOpen(UINT8 item);
extern	void	stationService();
extern	void	stationName(UINT8 item, UINT8 *n);
extern	void	stationAddr(UINT8 item, ipAddr *ip, UINT16 *port, char *url);
extern	void	stationProto(UINT8 item, UINT8 *proto);

#endif /* STATION_H_ */