/*
 * station.h
 *
 * Created: 2012-12-22 18:07:35
 *  Author: krzychu
 */ 


#ifndef STATION_H_
#define STATION_H_

//TODO: sound decoder handler

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
//************************************
// Method:    stationInit
// FullName:  Initializes station variables and state-machine
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
extern	void	stationInit();


//************************************
// Method:    stationClose
// FullName:  CLose current station
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
extern	void	stationClose();

//************************************
// Method:    stationOpen
// FullName:  Open selected station
// Access:    public 
// Returns:   Station machine state
// Qualifier:
// Parameter: UINT8 item
//************************************
extern	UINT8	stationOpen(UINT8 item);


//************************************
// Method:    stationService
// FullName:  Handles data feeding to decoder and maintainig buffer size
// Access:    public 
// Returns:   extern	void
// Qualifier:
//************************************
extern	void	stationService();


//************************************
// Method:    stationName
// FullName:  Gets station name
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: UINT8 item - number of station
// Parameter: UINT8 * n - pointer to name string storage
//************************************
extern	void	stationName(UINT8 item, UINT8 *n);


//************************************
// Method:    stationAddr
// FullName:  Get data necessary to connect to selected station
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: UINT8 item - number of station
// Parameter: ipAddr * ip - pointer to station IP address
// Parameter: UINT16 * port - pointer to station port
// Parameter: char * url - pointer to station URL address
//************************************
extern	void	stationAddr(UINT8 item, ipAddr *ip, UINT16 *port, char *url);


//************************************
// Method:    stationProto
// FullName:  stationProto
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: UINT8 item - number of station
// Parameter: UINT8 * proto - pointer to station protocol
//************************************
extern	void	stationProto(UINT8 item, UINT8 *proto);

#endif /* STATION_H_ */