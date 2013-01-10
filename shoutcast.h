/*
 * shoutcast.h
 *
 * Created: 2013-01-03 19:56:37
 *  Author: krzychu
 */ 
#include "types.h"
#include "main.h"
#include "station.h"

#ifndef SHOUTCAST_H_
#define SHOUTCAST_H_

// defines ----------------------------------------------------
#define SHOUTCAST_SERVERPORT	(8000)
#define SHOUTCAST_CLIENTPORT	(24001)

#define SHOUTCAST_TIMEOUT		(5)
#define SHOUTCAST_TRY			(0x03)

// machine-states ----------------------------------------------
#define SHOUTCAST_CLOSED		(1)
#define SHOUTCAST_CLOSE			(2)
#define SHOUTCAST_OPEN			(3)
#define SHOUTCAST_OPENED		(4)
#define SHOUTCAST_HEADER		(5)
#define SHOUTCAST_ERROR			(129)
#define SHOUTCAST_TIMEOUTERR	(130)

#define SHOUTCAST_TEST			(3)

// proto -------------------------------------------------------
UINT8	shoutcastOpen(UINT8 item);
void	shoutcastClose();
void	shoutcastData(UINT8 *data, UINT16 len);
void	shoutcastTcpApp(UINT8 index, UINT8 *data, UINT16 len);

#endif /* SHOUTCAST_H_ */