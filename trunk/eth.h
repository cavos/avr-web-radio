/*
 * eth.h
 *
 * Created: 2012-08-21 17:42:03
 *  Author: krzychu
 */ 


#ifndef ETH_H_
#define ETH_H_

#include "types.h"

#define HTONS(x)	((((x)&0x00FF)<<8)+(((x)&0xFF00)>>8))
#define HTONS32(x)	(((((x)&0xFF000000)>>24)|(((x)&0x00FF0000)>>8)|(((x)&0x0000FF00)<<8)|(((x)&0x000000FF)<<24)))

#define IP_BROADCAST		0xFFFFFFFF

#ifndef MAX_ARP_ENTRY
#define MAX_APP_ENTRY		5
#endif

#ifndef MAX_TCP_ENTRY
#define MAX_TCP_ENTRY		5
#endif

#ifndef MAX_ARP_ENTRY
#define MAX_ARP_ENTRY		5
#endif

#ifndef TCP_TIMEOFF
#define TCP_TIMEOFF			0xff
#endif

#ifndef ARP_TIMEOFF
#define ARP_TIMEOFF			0xff
#endif

#ifndef ARP_TIMEMAX
#define ARP_TIMEMAX			90
#endif

#ifndef MTU_SIZE
#define MTU_SIZE			1300
#endif

// union uesd to store MAC address, can be accessed either as 8- or 16bit array
typedef	union
{
	UINT8	b8[6];
	UINT16	b16[3];
} macAddr;

// union used for storing IP address, can be accessed either as 8- or 16bit array or one 32bit value
typedef	union
{
	UINT8	b8[4];
	UINT16	b16[2];
	UINT32	b32;
} ipAddr;

// struct for holding basic network information about device and gateway address
typedef	struct
{
	macAddr	mac;
	ipAddr	ipaddr;
	ipAddr	netmask;
	ipAddr	gateway;
}device;

// struct for holding information about TCP connections
typedef	struct  
{
	macAddr	mac;
	ipAddr	ip;
	UINT16	port;
	UINT16	localPort;
	UINT32	acknum;
	UINT32	seqnum;
	UINT8	flags;
	UINT8	status;
	UINT8	time;
	UINT8	error;
	UINT16	window;
	//UINT16	(*appCall)(UINT8*, UINT16);
} TCPtable;

// struct for holding information about UDP connections
typedef	struct
{
	macAddr mac;
	ipAddr ip;
	UINT16 localPort;
	UINT16 dstPort;
}UDPtable;

// strut for holding information about ARP connections
typedef	struct
{
	macAddr	mac;
	ipAddr	ip;
	UINT8	arpTime;
}ARPtable;

// ETHERTNET -------------------------------------------------
#define ETH_OFFSET		(0x00)
#define ETH_HEADER_SIZE	(14)
#define	ETH_TYPE_IP		(0x0800)
#define ETH_TYPE_ARP	(0x0806)

typedef	struct  
{
	macAddr	dstMac;
	macAddr	srcMac;
	UINT16	type;
} ETH_HEADER;

#define ETH_HW_ETH		0x0001

//	ARP	-------------------------------------------------------
#define ARP_OFFSET		0x0E
#define	ARP_HEADER_SIZE	28

typedef struct
{
	UINT16	hwType;		//hardware type
	UINT16	prType;		//protocol type
	UINT8	hwLength;	//hardware size
	UINT8	prLength;	//protocol size
	UINT16	opCode;		//operation code
	macAddr	sourceMac;	//sender mac address
	ipAddr	sourceIP;	//sender ip address
	macAddr	targetMac;	//destination mac address
	ipAddr	targetIP;	//destination ip address
}ARP_HEADER;

#define ARP_REQUEST		0x0100
#define ARP_REPLY		0x0200
#define ARP_REPLY_LEN	60
#define ARP_REQUEST_LEN	42

//	IP	------------------------------------------------------
#define IP_OFFSET		(0x0E)
#define IP_HEADER_SIZE	(20)
#define IP_PR_ICMP		(0x01)
#define IP_PR_TCP		(0x06)
#define IP_PR_UDP		(0x11)

typedef struct
{
	UINT8	ver_len;	//4b version + 4b length in 32b words
	UINT8	tos;		//type of service
	UINT16	length;		//packet length
	UINT16	id;			//id
	UINT16	flags_offset;// 3b flags + 13b offset
	UINT8	ttl;		//time to live
	UINT8	protocol;	// protocol
	UINT16	checksum;	// checksum of IP header
	ipAddr	sourceIP;
	ipAddr	targetIP;
}IP_HEADER;

//	ICMP	----------------------------------------------------------
#define ICMP_OFFSET			0x22
#define ICMP_HEADER_SIZE	4
#define	ICMP_TYPE_ECHO_REQ	0x08
#define ICMP_TYPE_ECHO_REP	0x00

typedef struct
{
	UINT8	type;	//type
	UINT8	code;	//code
	UINT16	checksum;//checksum
	UINT8	data;
}ICMP_HEADER;

#define ICMP_REPLY_LEN		74

//	TCP	--------------------------------------------------------------
#define TCP_OFFSET		0x22
#define TCP_HEADER_SIZE	(20)
#define TCP_DATA		(ETH_HEADER_SIZE+IP_HEADER_SIZE+TCP_HEADER_SIZE)	
#define TCP_TIMEOUT		3
#define TCP_MAXERROR	5
#define TCP_FLAG_FIN	0x01
#define TCP_FLAG_SYN	0x02
#define TCP_FLAG_RST	0x04
#define TCP_FLAG_PSH	0x08
#define TCP_FLAG_ACK	0x10

typedef struct
{
	UINT16	srcPort;		//source port
	UINT16	dstPort;		//destination port
	UINT32	seqNum;			//sequence number
	UINT32	ackNum;			//acknowledgement number
	UINT8	length;			//length
	UINT8	flags;			//flags
	UINT16	window;			//window
	UINT16	checksum;		//checksum
	UINT16	urgentPtr;		//urgent pointer
}TCP_HEADER;

#define TCP_WINDOW		(MTU_SIZE-100)
#define TCP_S_CLOSED	0
#define	TCP_S_OPENED	1
#define	TCP_S_OPEN		2
#define TCP_S_ABORT		3
#define TCP_S_CLOSE		4
#define	TCP_S_FINISH	5

// udp ------------------------------------------------------
#define UDP_OFFSET		(0x22)
//#define UDP_OFFSET		(ETH_HEADER_SIZE+IP_HEADER_SIZE)
#define UDP_HEADER_SIZE 8
#define UDP_DATA		(0x2A)
#define UDP_MAX_ENTRIES 5
#define UDP_DEBUG		5

typedef struct
{
	UINT16	srcPort;	// source port
	UINT16	dstPort;	// destination port
	UINT16	lenght;
	UINT16	checksum;
}UDP_HEADER;

// struct pointers ------------------------------------------
extern	ETH_HEADER	*eth;
extern	ARP_HEADER	*arp;
extern	IP_HEADER	*ip;
extern	ICMP_HEADER	*icmp;
extern	TCP_HEADER	*tcp;
extern	UDP_HEADER	*udp;

extern	device		settings;
extern	TCPtable	tcpTable[MAX_TCP_ENTRY];
extern	TCPtable	*tcpConn;
extern	UDPtable	udpTable[UDP_MAX_ENTRIES+1];
extern	ARPtable	arpTable[MAX_ARP_ENTRY];
extern	UINT8		packetBuffer[MTU_SIZE+1];

extern	UINT8		udpDoChecksum;
#define UDP_CHECKSUM_ON	255
#define UDP_CHECKSUM_OFF		0


// user must program it!
//extern void	nicSend();
//extern UINT16	nicReceive();


// prototypes ----------------------------------------------------
///<summary>
/// Initializes TCP/IP stack, gets MAC address of gateway
///</summary>
extern	void	ethInit();

///<summary>
/// Function used to process incoming packets, must be called within main loop
///</summary>
extern	void	ethService();

///<summary>
/// Used to perform periodical actions such as refreshing connection tables or attempting connection retries
/// must be called periodically, call interval 1 second
///</summary>
extern	void	ethTimeService();

///<summary>
/// Search ARP table for requested IP address
/// return index to ARP table with MAC address
///</summary>
extern	UINT8	arpEntrySearch(ipAddr ipaddr);

///<summary>
/// Translates IP address to MAC address
/// return index to ARP table with MAC address
///</summary>
extern	UINT8	arpRequest(ipAddr ipaddr);

///<summary>
/// Routine used to process ICMP packets, restricted to replying for ECHO packets
///</summary>
extern	void	icmpService();

///<summary>
/// Routine used to process TCP packets
///</summary>
extern	void	tcpService();

///<summary>
/// Method used to clean-up TCP table, called periodically
///</summary>
extern	void	tcpTimeService();

///<summary>
/// Function used to connect to specified server
/// ipAddr targetIp - ip address of server
/// UINT16 dstPort - server port
/// UINT16 srcPort - local port
/// return index of this connection in TCP table, MAX_TCP_ENTRY on error or table full
///</summary>
extern	UINT8	tcpConnect(ipAddr targetIp, UINT16 dstPort, UINT16 srcPort, UINT16 window);

///<summary>
/// Listens to selected port and creates connection if SYN packet arrives
/// UINT16 port - listen port
/// return index of this connection in TCP table, MAX_TCP_ENTRY on error or table full
///</summary>
extern	UINT8	tcpListen(UINT16 port);

///<summary>
/// Close selected connection, send FIN
///</summary>
extern	void	tcpClose(UINT8 index);

///<summary>
/// Aborts connection
///</summary>
extern	void	tcpAbort(UINT8 index);

///<summary>
/// Sends data
///</summary>
extern	void	tcpSend(UINT8 index, UINT16 len);

extern	void	udpDbgSend(const char *data, UINT16 len);
//extern	void	udpSend(UINT8 index);

///<summary>
/// Make ethernet header
/// ipAddr targetIp - ip address of server
///</summary>
extern	void	ethMakeHeader(ipAddr targetIp);

///<summary>
/// Make IPv4 header
/// ipAddr targetIp - ip address of server
///</summary>
extern	void	ipMakeHeader(ipAddr	targetIp);

///<summary>
/// Make TCP header
/// UINT8 index - connection number in TCP table
/// UINT16 len - length of data
///</summary>
extern	void	tcpMakeHeader(UINT8 index, UINT16 len);

///<summary>
/// Make UDP header
/// UINT8 index - connection number in UDP table
/// UINT16 len - length of data
///</summary>
extern	void	udpMakeHeader(UINT8 index, UINT16 len);


extern	UINT16	ipChecksum();
extern	UINT16  tcpChecksum(UINT8 index, UINT16 datalength); // computes only tcp pseudo-header checksum
extern	UINT16  udpChecksum(UINT8 index); // computes only udp pseudo-header checksum
extern  UINT16	checksum(UINT8 *data, UINT16 len);

extern	UINT16	htons(const UINT16 val);
extern	UINT32	htons32(const UINT32 val);

//-------------------------------------
extern	void	arpAddEntry();
extern	void	arpReply();
extern	void	arpTimeService();

extern	void	tcpSClose(UINT8 index);
extern	void	tcpSOpen(UINT8 index);
extern	void	tcpSOpened(UINT8 index);
extern	void	tcpSAbort(UINT8 index);
extern	void	tcpSFinish(UINT8 index);

#endif /* ETH_H_ */