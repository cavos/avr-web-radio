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

//TODO: icy parser

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

//************************************
// Method:    shoutcastOpen
// FullName:  Opens selected station
// Access:    public 
// Returns:   machine-state code
// Qualifier:
// Parameter: UINT8 item - number of station
//************************************
UINT8	shoutcastOpen(UINT8 item);

//************************************
// Method:    shoutcastClose
// FullName:  Close current station
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void	shoutcastClose();

///<summary>
/// do not use
///</summary>
void	shoutcastData(UINT8 *data, UINT16 len);

//************************************
// Method:    shoutcastTcpApp
// FullName:  Shoutcast tcp application handler
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: UINT8 index - index of connection within TCP table
// Parameter: UINT8 * data - pointer to shoutcast data
// Parameter: UINT16 len - length of data
//************************************
void	shoutcastTcpApp(UINT8 index, UINT8 *data, UINT16 len);

//************************************
// Method:    shoutcastBuffer
// FullName:  Move received data to buffer, handle stationService()
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: UINT8 * data - pointer to music data
// Parameter: UINT16 len - length of data
//************************************
void	shoutcastBuffer(UINT8 *data, UINT16 len);

///<summary>
/// do not use
///</summary>
void	shoutcastPlay();

#endif /* SHOUTCAST_H_ */