/*
 * ip_icmp_arp.c
 *
 * Created: 2012-08-22 09:38:29
 *  Author: krzychu
 */ 
#define	F_CPU	12228000UL

#include <util/delay.h>
#include <avr/io.h>
#include "main.h"
#include "types.h"
#include "enc28j60.h"
//#include "checksum.h"
#include "eth.h"


ARPtable	arpTable[MAX_ARP_ENTRY];
ARP_HEADER	*arp = (ARP_HEADER*)&packetBuffer[ARP_OFFSET];
ICMP_HEADER	*icmp = (ICMP_HEADER*)&packetBuffer[ICMP_OFFSET];
IP_HEADER	*ip	= (IP_HEADER*)&packetBuffer[IP_OFFSET];


void	ipMakeHeader(ipAddr targetIp)
{
	ethMakeHeader(targetIp);
	eth->type = HTONS(ETH_TYPE_IP);
	
	ip->id = 0x0000;
	ip->ver_len = 0x40|(IP_HEADER_SIZE>>2);
	ip->tos = 0;
	ip->flags_offset = HTONS(0x4000); // do not fragment
	ip->ttl = 128;
	ip->checksum = 0;
	ip->targetIP.b32 = HTONS32(targetIp.b32);
	ip->sourceIP.b32 = HTONS32(settings.ipaddr.b32);
	
	ip->checksum = HTONS(ipChecksum());
}

void	arpTimeService()
{
	for (UINT8 i = 0; i < MAX_ARP_ENTRY; i++)
	{
		if( arpTable[i].ip.b32 != 0 && arpTable[i].arpTime == 0)
		{
			//free entry
			arpTable[i].ip.b32 = 0;
		}
		else if (arpTable[i].ip.b32 != 0 && arpTable[i].arpTime != ARP_TIMEOFF)
		{
			arpTable[i].arpTime--;
		}
	}
}

void	arpAddEntry()
{
	// check if already listed
	for(UINT8 i = 0; i < MAX_ARP_ENTRY; i++)
	{
		if(eth->type == HTONS(ETH_TYPE_IP))
		{
			if(arpTable[i].ip.b32 == HTONS32(ip->sourceIP.b32))
			{
				if(arpTable[i].arpTime != ARP_TIMEOFF)
				{
					arpTable[i].arpTime = ARP_TIMEMAX;
				}
			}
		}
		if(eth->type == HTONS(ETH_TYPE_ARP))
		{
			if(arpTable[i].ip.b32 == HTONS32(arp->sourceIP.b32))
			{
				if (arpTable[i].arpTime != ARP_TIMEOFF)
				{
					arpTable[i].arpTime = ARP_TIMEMAX;
				}
			}
		}
	}
	
	// find free entry
	for(UINT8 i = 0; i < MAX_ARP_ENTRY; i++)
	{
		if(arpTable[i].ip.b32 == 0)
		{
			if (eth->type == HTONS(ETH_TYPE_IP))
			{
				for(UINT8 y = 0; y < 6; y++)
					{arpTable[i].mac.b8[y] = eth->srcMac.b8[y];}
				arpTable[i].ip.b32 = HTONS32(ip->sourceIP.b32);
				arpTable[i].arpTime = ARP_TIMEMAX;
				return;
			}
			if(eth->type == HTONS(ETH_TYPE_ARP))
			{
				for (UINT8 y=0; y<6; y++)
				{
					arpTable[i].mac.b8[y] = arp->sourceMac.b8[y];
				}
				arpTable[i].ip.b32 = HTONS32(arp->sourceIP.b32);
				arpTable[i].arpTime = ARP_TIMEMAX;
			}
		}
	}
	
}

UINT8	arpEntrySearch(ipAddr ipaddr)
{
	// check routing
	if( (ipaddr.b32 & settings.netmask.b32) != (settings.gateway.b32 & settings.netmask.b32))
	{
		ipaddr.b32 = settings.gateway.b32; // use router
	}
	
	//LED_ON();
	// look for mac in arp table
	for(UINT8 i = 0; i < MAX_ARP_ENTRY; i++)
	{
		if( ipaddr.b32 == arpTable[i].ip.b32)
		{
			return i;
		}
	}
	return MAX_ARP_ENTRY;
}

void	arpReply()
{
	if((arp->hwType == HTONS(ETH_HW_ETH)) && (arp->prType == HTONS(ETH_TYPE_IP)) && (HTONS32(arp->targetIP.b32) == settings.ipaddr.b32))
		{
			//LED_ON();
			if(arp->opCode == ARP_REQUEST)
			{
				
				for (UINT8 i = 0; i < 6; i++)
				{
					eth->dstMac.b8[i] = eth->srcMac.b8[i];
					eth->srcMac.b8[i] = settings.mac.b8[i];
					arp->targetMac.b8[i] = eth->dstMac.b8[i];
					arp->sourceMac.b8[i] = settings.mac.b8[i];
				}
				// set opcode
				arp->opCode = ARP_REPLY;
				arp->targetIP.b32 = arp->sourceIP.b32;
				arp->sourceIP.b32 = HTONS32(settings.ipaddr.b32);
				
				enc28j60_sendPacket(ARP_REPLY_LEN, packetBuffer, 0,0);
			}
			else if(arp->opCode == ARP_REPLY)
			{
				//LED_ON();
				arpAddEntry();
			}
			
		}
}

UINT8	arpRequest(ipAddr ipaddr)
{
	UINT8 f = arpEntrySearch(ipaddr);
	if (f < MAX_ARP_ENTRY)
	{
		return f;
	}
	
	ipAddr dstIPcpy;
	dstIPcpy.b32 = ipaddr.b32;
	
	// check routing
	if( (ipaddr.b32 & settings.netmask.b32) != (settings.ipaddr.b32 && settings.netmask.b32))
	{
		ipaddr.b32 = settings.gateway.b32;
	}
	
	eth->type = HTONS(ETH_TYPE_ARP);
	ethMakeHeader(ipaddr);
	arp->sourceIP.b32 = HTONS32(settings.ipaddr.b32);
	arp->targetIP.b32 = HTONS32(ipaddr.b32);
	
	for(UINT8 i = 0; i < 6; i++)
	{
		arp->sourceMac.b8[i] = settings.mac.b8[i];
		arp->targetMac.b8[i] = 0x00;
	}
	arp->hwType = HTONS(ETH_HW_ETH);
	arp->prType = HTONS(ETH_TYPE_IP);
	arp->hwLength = 6;
	arp->prLength = 4;
	arp->opCode = ARP_REQUEST;
	
	enc28j60_sendPacket(42, packetBuffer,0,0);
	
	//wait for reply
	for(UINT8 i = 0; i < 15; i++)
	{
		_delay_ms(20);
		
		ethService();
		UINT8 index = arpEntrySearch(ipaddr);
		UINT8 index2 = arpEntrySearch(dstIPcpy);
		if( (index < MAX_ARP_ENTRY) || (index2 < MAX_ARP_ENTRY))
			return (index < MAX_ARP_ENTRY)?index:index2;
	}
	
	return 0;
}


void	icmpService(/*ipAddr dstIp, UINT8 type*/)
{
	UINT32 tmp;
	if(icmp->type == ICMP_TYPE_ECHO_REQ)
	{
		
		
		//LED_ON();
		ip->length = HTONS(ICMP_REPLY_LEN);
		ip->protocol = IP_PR_ICMP;
		ip->sourceIP.b32 = HTONS32(ip->sourceIP.b32);
		ipMakeHeader(ip->sourceIP); // error probability with copying ip addr!
		// TODO: checksum!!!
		icmp->type = 0;
		icmp->code = 0;
		tmp = HTONS(icmp->checksum); // only the type is changed 
		tmp = tmp + 0x0800; // so add 8
		icmp->checksum = (tmp&0xFFFF0000)>>16;
		icmp->checksum = tmp + icmp->checksum;
		icmp->checksum = HTONS(icmp->checksum);
		
		enc28j60_sendPacket(ICMP_REPLY_LEN, packetBuffer, 0,0);
	}

}