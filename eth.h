/*
 * eth.h
 *
 * Created: 2012-08-21 17:42:03
 *  Author: krzychu
 */ 


#ifndef ETH_H_
#define ETH_H_

#include "types.h"

#define HTONS(x)	(((x)&0x00FF)<<8)+(((x)&0xFF00)>>8)
#define HTONS32(x)	(((x)&0xFF000000)>>24)|(((x)&0x00FF0000)>>8)|(((x)&0x0000FF00)<<8)|(((x)&0x000000FF)<<24)

#define IP_BROADCAST		0xFFFFFFFF

#define MAX_APP_ENTRY		5
#define MAX_TCP_ENTRY		5
#define MAX_ARP_ENTRY		5
#define TCP_TIMEOFF			0xff
#define ARP_TIMEOFF			0xff
#define ARP_TIMEMAX			90
#define MTU_SIZE			1000

//typedef	UINT16	port_t;

typedef	union
{
	UINT8	b8[6];
	UINT16	b16[3];
} macAddr;

typedef	union
{
	UINT8	b8[4];
	UINT32	b32;
} ipAddr;

typedef	struct
{
	macAddr	mac;
	ipAddr	ipaddr;
	ipAddr	netmask;
	ipAddr	gateway;
}device;

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
	UINT8	firstAck:1;
} TCPtable;

typedef	struct
{
	macAddr	mac;
	ipAddr	ip;
	UINT8	arpTime;
}ARPtable;

// ETHERTNET -------------------------------------------------
#define ETH_OFFSET		0x00
#define ETH_HEADER_SIZE	14
#define	ETH_TYPE_IP		0x0800
#define ETH_TYPE_ARP	0x0806

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
#define IP_OFFSET		0x0E
#define IP_HEADER_SIZE	20
#define IP_PR_ICMP		0x01
#define IP_PR_TCP		0x06
#define IP_PR_UDP		0x11

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
#define TCP_HEADER_SIZE	20
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
	UINT16	dstPortl;		//destination port
	UINT32	seqNum;			//sequence number
	UINT32	ackNum;			//acknowledgement number
	UINT8	length;			//length
	UINT8	flags;			//flags
	UINT16	window;			//window
	UINT16	checksum;		//checksum
	UINT16	urgentPtr;		//urgent pointer
}TCP_HEADER;

#define TCP_WINDOW		(MTU_SIZE-100)
#define TCP_S_CONNECTED	1
#define TCP_S_ACKED		2
#define	TCP_S_NEWDATA	3
#define TCP_S_REXMIT	4
#define TCP_S_POOL		5
#define TCP_S_CLOSED	6
#define TCP_S_ABORTED	7
#define TCP_S_TIMEDOUT	8


// struct pointers ------------------------------------------
extern	ETH_HEADER	*eth;
extern	ARP_HEADER	*arp;
extern	IP_HEADER	*ip;
extern	ICMP_HEADER	*icmp;
extern	TCP_HEADER	*tcp;

extern	device		settings;
//extern	TCPtable	tcpTable[MAX_TCP_ENTRY+1];
//extern	TCPtable	*tcpConn;
extern	ARPtable	arpTable[MAX_ARP_ENTRY];
extern	UINT8		packetBuffer[MTU_SIZE+1];


// user must program it!
//extern void nicSend(sd);


// prototypes ----------------------------------------------------
extern	void	ethInit();
//extern	void	ethArp();
extern	void	ethGetData();

extern	UINT8	arpEntrySearch(ipAddr ipaddr);
extern	UINT8	arpRequest(ipAddr ipaddr);

extern	void	icmpService();

//extern	void	tcpMakePacket(UINT16 len);
//extern	void	tcpConnectTo(ipAddr targetIp, UINT16 targetPort);
//extern	void	tcpListenTo(UINT16 listenPort);
//extern	void	tcpDeleteConn();
//extern	void	tcpSend(UINT16 len);
//extern	void	tcpAppChangePort(UINT16 old, UINT16 new);

extern	void	ethMakeHeader(ipAddr targetIp);
extern	void	ipMakeHeader(ipAddr	targetIp);

extern	UINT16	ipChecksum(); //proper computation?

extern	UINT16	htons(const UINT16 val);
extern	UINT32	htons32(const UINT32 val);

//-------------------------------------
extern	void	ethService();
extern	void	arpAddEntry();
extern	void	arpReply();
extern	void	arpTimeService();
//extern	void	tcpTimeService();
//extern	void	tcpService();
//extern	UINT8	tcpSearchPort(UINT16 p);

#endif /* ETH_H_ */