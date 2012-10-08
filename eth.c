/*
 * eth.c
 *
 * Created: 2012-08-30 12:56:04
 *  Author: krzychu
 */ 

#define	F_CPU	12228000UL

#include "eth.h"
#include "types.h"
#include "main.h"
#include "enc28j60.h"
#include <util/delay.h>
#include <avr/wdt.h>

UINT8		packetBuffer[MTU_SIZE+1];
ETH_HEADER	*eth = (ETH_HEADER*)&packetBuffer[ETH_OFFSET];

UINT32	htons32(const UINT32 val)
{
	return	HTONS32(val);
}

UINT16	htons(const UINT16 val)
{
	return	HTONS(val);
}


void	ethService()
{
	// check packet type
	if (eth->type == HTONS(ETH_TYPE_ARP))
	{
		arpReply();
	}
	else
	{
		if (eth->type == HTONS(ETH_TYPE_IP))
		{
			if((settings.ipaddr.b32 == HTONS32(ip->targetIP.b32)) || (HTONS32(ip->targetIP.b32) == IP_BROADCAST))
			{
				arpAddEntry();
				if (ip->protocol == IP_PR_ICMP)
				{
					icmpService();
				}
				else if (ip->protocol == IP_PR_TCP)
				{
					//tcpService();
				}
				else if (ip->protocol == IP_PR_UDP)
				{
					//udpService();
				}
			}
		}
	}
}

void	ethGetData()
{
	UINT16 length;
	
	length = enc28j60_receivePacket(MTU_SIZE, packetBuffer);
	if (length > 0)
	{
		ethService();
	}
}

void	ethMakeHeader(ipAddr targetIp)
{
	UINT8 j;
	
	// check routing
	if( (targetIp.b32 & settings.netmask.b32) != (settings.ipaddr.b32 & settings.netmask.b32))
	{
		targetIp.b32 = settings.gateway.b32;
	}
	
	// search for MAC addr in ARP table
	j = arpEntrySearch(targetIp);
	
	if (j != MAX_ARP_ENTRY)
	{
		for(UINT8 i = 0; i < 6 ;i++ )
		{
			eth->dstMac.b8[i] = arpTable[j].mac.b8[i];
			eth->srcMac.b8[i] = settings.mac.b8[i];
		}
		return;
	}
	for(UINT8 i = 0; i < 6; i++)
	{
		eth->dstMac.b8[i] = 0xFF;
		eth->srcMac.b8[i] = settings.mac.b8[i];
	}
}

void	ethInit()
{
	//ethArp();
	while(1)
	{
		UINT8 j = arpEntrySearch(settings.gateway);
		
		if (j != MAX_ARP_ENTRY)
		{
			arpTable[j].arpTime = ARP_TIMEOFF;
			break;
		}
		else
		{
			arpRequest(settings.gateway);
			_delay_ms(500);
			wdt_reset();
		}
		
		UINT16	length = enc28j60_receivePacket(MTU_SIZE,packetBuffer);
		if(length > 0)
		{
			if (eth->type == HTONS(ETH_TYPE_ARP))
			{
				arpReply();
			}
		}
	}
}

UINT16	ipChecksum()
{
	UINT32	sum32 = 0;
	UINT16	result16 = 0;
	UINT16	tmp = (ip->ver_len)<<8;
	
	sum32 = sum32 + tmp + ip->tos;
	sum32 = sum32 + HTONS(ip->length);
	sum32 = sum32 + HTONS(ip->flags_offset);
	tmp = (ip->ttl)<<8;
	sum32 = sum32 + tmp + ip->protocol;
	sum32 = sum32 + ((HTONS32(ip->sourceIP.b32))>>16); // msb
	sum32 = sum32 + ((HTONS32(ip->sourceIP.b32))&0x0000FFFF); //lsb
	sum32 = sum32 + ((HTONS32(ip->targetIP.b32))>>16); // msb
	sum32 = sum32 + ((HTONS32(ip->targetIP.b32))&0x0000FFFF); // lsb
	
	tmp = (sum32&0xFFFF0000)>>16;
	sum32 = sum32 + tmp;
	//sum32 = (sum32&0x0000FFFF) + ((sum32&0xFFFF0000)>>16); // might carry
	//result16 = ((sum32&0x0000FFFF) + ((sum32&0xFFFF0000)>>16))&0xFFFF; // add carry

	result16 = sum32&0x0000FFFF;
	result16 = ~result16;
	
	return result16;
}