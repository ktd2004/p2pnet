/* Copyright 2014 by Lee yong jun
 * All rights reserved
 *
 * Distribute freely, except: don't remove my name from the source or
 * documentation (don't take credit for my work), mark your changes (don't
 * get me blamed for your possible bugs), don't alter or remove this
 * notice.  May be sold if buildable source is provided to buyer.  No
 * warrantee of any kind, express or implied, is included with this
 * software; use at your own risk, responsibility for damages (if any) to
 * anyone resulting from the use of this software rests entirely with the
 * user.
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Lee yong jun          iamhere01@naver.com
 */
#pragma once

#include <NetLink.h>
#include <FastList.h>

typedef unsigned long	PKTSEQ;

#pragma pack(push)
#pragma pack(1)

//< p2p packet command
enum eP2PPacketType
{
	eP2P_SYNC = 2,					// sync packet
	eP2P_SYNC_ACK,					// sync ack packet
	eP2P_KEEPING_CONNECTION,		// keep connection
	eP2P_RELIABLE_ACK,				// reliable ack
	eP2P_JOIN,						// join rendezvous
	eP2P_JOIN_ACK,					// join ack rendezvous
	eP2P_LEAVE,						// leave rendezvous
	eP2P_LEAVE_ACK,					// leave ack renendezvous

	eP2P_RELIABLE_SEND_TO,			// reliable packet
	eP2P_SEND_TO,					// unreliable packet
	eP2P_PKT_MAX,
};

//< p2p packet header
struct P2PNET_PACKET_BASE : listnode(P2PNET_PACKET_BASE)
{
	SOCKADDR_IN			Addr;		// send address
	unsigned int		iLen;		// packet length
	unsigned short		iPackID;	// packet id
	PKTSEQ				iPktSeq;	// reliable/unreliable packet seq
	unsigned int		iCellID;	// cell id in rendezvous 
	unsigned long		iTransTick;	// send time
	TNID				iFromID;	// sender id
	TNID				iToID;		// receiver id ( only rendezvous )

	P2PNET_PACKET_BASE()
		: iLen(sizeof(P2PNET_PACKET_BASE))
		, iPackID(0)
		, iPktSeq(0)
		, iTransTick(0)
		, iFromID(0)
		, iToID(0)
	{
		memset( &Addr, 0x00, sizeof(Addr) );
	}
};

typedef util::list<P2PNET_PACKET_BASE>	P2PNET_PACKET_LIST;
#define PKT_DATA_POS(pkt)				((char*)pkt) + sizeof(P2PNET_PACKET_BASE)
#define PKT_DATA_LEN(pkt)				(pkt->iLen - (sizeof(P2PNET_PACKET_BASE) - sizeof(SOCKADDR_IN)))
#define PKT_LEN(pkt)					(sizeof(pkt) - sizeof(SOCKADDR_IN))

//< sync packet
struct P2P_SYNC : public P2PNET_PACKET_BASE
{
	P2P_SYNC(unsigned int iCellID, TNID iFromID)
	{
		this->iLen		= PKT_LEN(P2P_SYNC);
		this->iPackID	= eP2P_SYNC;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
	}
};

//< sync ack packet
struct P2P_SYNC_ACK : public P2PNET_PACKET_BASE
{
	unsigned long	iSelfAddr;
	unsigned short	iSelfPort;
	P2P_SYNC_ACK(unsigned int iCellID, TNID iFromID, unsigned long iTransTick, const SOCKADDR_IN& addr)
	{
		this->iLen			= PKT_LEN(P2P_SYNC_ACK);
		this->iPackID		= eP2P_SYNC_ACK;
		this->iCellID		= iCellID;
		this->iFromID		= iFromID;
		this->iTransTick	= iTransTick;
		this->iSelfAddr		= addr.sin_addr.S_un.S_addr;
		this->iSelfPort		= addr.sin_port;
	}
};

//< keep connection packet
struct P2P_KEEP_CONNECTION : public P2PNET_PACKET_BASE
{
	P2P_KEEP_CONNECTION(unsigned int iCellID, TNID iFromID)
	{
		this->iLen		= PKT_LEN(P2P_KEEP_CONNECTION);
		this->iPackID	= eP2P_KEEPING_CONNECTION;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
	}
};


//< reliable ack packet
struct P2P_RELIABLE_ACK : public P2PNET_PACKET_BASE
{    
	P2P_RELIABLE_ACK(PKTSEQ iPktSeq, unsigned int iCellID, TNID iFromID, TNID iToID, unsigned long iTransTick)
	{
		this->iLen			= PKT_LEN(P2P_RELIABLE_ACK);
		this->iPackID		= eP2P_RELIABLE_ACK;
		this->iPktSeq		= iPktSeq;
		this->iCellID		= iCellID;
		this->iFromID		= iFromID;
		this->iToID			= iToID;
		this->iTransTick	= iTransTick;
	}
};


//< join packet
struct P2P_JOIN : public P2PNET_PACKET_BASE
{
	Network_IF			NIF;
	P2P_JOIN(unsigned int iCellID, TNID iFromID, const Network_IF& nif)
	{
		this->iLen		= PKT_LEN(P2P_JOIN);
		this->iPackID	= eP2P_JOIN;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
		memcpy( &NIF, &nif, sizeof(Network_IF) );
	}
};

//< join ack packet
struct P2P_JOIN_ACK : public P2PNET_PACKET_BASE
{
	P2P_JOIN_ACK(PKTSEQ, unsigned int iCellID, TNID iFromID, unsigned long iTransTick)
	{
		this->iLen			= PKT_LEN(P2P_JOIN_ACK);
		this->iPackID		= eP2P_JOIN_ACK;
		this->iPktSeq		= iPktSeq;
		this->iCellID		= iCellID;
		this->iFromID		= iFromID;
		this->iTransTick	= iTransTick;
	}
};


//< leave packet
struct P2P_LEAVE : public P2PNET_PACKET_BASE
{
	Network_IF	NIF;
	P2P_LEAVE(unsigned int iCellID, TNID iFromID, const Network_IF& nif)
	{
		this->iLen		= PKT_LEN(P2P_LEAVE);
		this->iPackID	= eP2P_LEAVE;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
		memcpy( &NIF, &nif, sizeof(Network_IF) );
	}
};

//< leave ack packet
struct P2P_LEAVE_ACK : public P2PNET_PACKET_BASE
{
	P2P_LEAVE_ACK(PKTSEQ iPktSeq, unsigned int iCellID, TNID iFromID, unsigned long iTransTick)
	{
		this->iLen			= PKT_LEN(P2P_LEAVE_ACK);
		this->iPackID		= eP2P_LEAVE_ACK;
		this->iPktSeq		= iPktSeq;
		this->iCellID		= iCellID;
		this->iFromID		= iFromID;
		this->iTransTick	= iTransTick;
	}
};
#pragma pack(pop)
