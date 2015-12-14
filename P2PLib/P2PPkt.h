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

#pragma pack(push)
#pragma pack(1)
//////////////////////////////////////////////////////////////////////////////////////
// P2P 패킷의 명령어.
enum eP2PPacketType
{
	eP2P_SYNC = 2,					// Sync 패킷
	eP2P_SYNC_ACK,					// Sync 패킷에 대한 ACK
	eP2P_KEEPING_CONNECTION,		// 홀펀칭 상태 유지 패킷
	eP2P_RELIABLE_ACK,				// Reliable 패킷에 대한 Ack
	eP2P_JOIN,						// 랑데뷰서버에 조인
	eP2P_JOIN_ACK,					// 랑데뷰서버에 조인에 대한 Ack
	eP2P_LEAVE,						// 랑데뷰서버에 연결종료
	eP2P_LEAVE_ACK,					// 랑데뷰서버에 연결종료

	eP2P_RELIABLE_SEND_TO,			// Reliable 패킷 전송
	eP2P_SEND_TO,					// 패킷 전송
	eP2P_PKT_MAX,
};

struct P2PNET_PACKET_BASE : listnode(P2PNET_PACKET_BASE)
{
	SOCKADDR_IN			Addr;		// 전송 주소
	unsigned int		iLen;		// 전체 길이 - 항상 이 자리에 2BYTE 만 허용
	unsigned short		iPackID;	// P2P 패킷 ID
	unsigned long		iPktSeq;	// Sequence ID : UDP, RUDP 패킷의 고유 번호
	unsigned int		iCellID;	// 홀펀칭 범위
	unsigned long		iTransTick;	// 전송시간
	unsigned int		iFromID;	// 발신자 고유 ID
	unsigned int		iToID;		// 수신자 고유 ID ( 릴레이에서만 사용 )

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


//////////////////////////////////////////////////////////////////////////////////////
// Sync 패킷
struct P2P_SYNC : public P2PNET_PACKET_BASE
{
	P2P_SYNC(unsigned int iCellID, unsigned int iFromID)
	{
		this->iLen		= PKT_LEN(P2P_SYNC);
		this->iPackID	= eP2P_SYNC;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
	}
};

//////////////////////////////////////////////////////////////////////////////////////
// Sync 패킷에 대한 Ack
struct P2P_SYNC_ACK : public P2PNET_PACKET_BASE
{
	P2P_SYNC_ACK(unsigned int iCellID, unsigned int iFromID)
	{
		this->iLen		= PKT_LEN(P2P_SYNC_ACK);
		this->iPackID	= eP2P_SYNC_ACK;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
	}
};

//////////////////////////////////////////////////////////////////////////////////////
// 연결 유지
struct P2P_KEEP_CONNECTION : public P2PNET_PACKET_BASE
{
	P2P_KEEP_CONNECTION(unsigned int iCellID, unsigned int iFromID)
	{
		this->iLen		= PKT_LEN(P2P_KEEP_CONNECTION);
		this->iPackID	= eP2P_KEEPING_CONNECTION;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
	}
};

//////////////////////////////////////////////////////////////////////////////////////
// Reliable 패킷에 대한 ACK
struct P2P_RELIABLE_ACK : public P2PNET_PACKET_BASE
{    
	P2P_RELIABLE_ACK(unsigned int iPktSeq, unsigned int iCellID, unsigned int iFromID, unsigned int iToID)
	{
		this->iLen		= PKT_LEN(P2P_RELIABLE_ACK);
		this->iPackID	= eP2P_RELIABLE_ACK;
		this->iPktSeq	= iPktSeq;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
		this->iToID		= iToID;
	}
};

//////////////////////////////////////////////////////////////////////////////////////
// JOIN 패킷
struct P2P_JOIN : public P2PNET_PACKET_BASE
{
	Network_IF	NIF;
	P2P_JOIN(unsigned int iCellID, unsigned int iFromID, const Network_IF& nif)
	{
		this->iLen		= PKT_LEN(P2P_JOIN);
		this->iPackID	= eP2P_JOIN;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
		memcpy( &NIF, &nif, sizeof(Network_IF) );
	}
};

//////////////////////////////////////////////////////////////////////////////////////
// JOIN 패킷에 대한 ACK
struct P2P_JOIN_ACK : public P2PNET_PACKET_BASE
{
	P2P_JOIN_ACK(unsigned int iPktSeq, unsigned int iCellID, unsigned int iFromID)
	{
		this->iLen		= PKT_LEN(P2P_JOIN_ACK);
		this->iPackID	= eP2P_JOIN_ACK;
		this->iPktSeq	= iPktSeq;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
	}
};

//////////////////////////////////////////////////////////////////////////////////////
// LEAVE 패킷
struct P2P_LEAVE : public P2PNET_PACKET_BASE
{
	Network_IF	NIF;
	P2P_LEAVE(unsigned int iCellID, unsigned int iFromID, const Network_IF& nif)
	{
		this->iLen		= PKT_LEN(P2P_LEAVE);
		this->iPackID	= eP2P_LEAVE;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
		memcpy( &NIF, &nif, sizeof(Network_IF) );
	}
};

//////////////////////////////////////////////////////////////////////////////////////
// LEAVE 패킷에 대한 ACK
struct P2P_LEAVE_ACK : public P2PNET_PACKET_BASE
{
	P2P_LEAVE_ACK(unsigned int iPktSeq, unsigned int iCellID, unsigned int iFromID)
	{
		this->iLen		= PKT_LEN(P2P_LEAVE_ACK);
		this->iPackID	= eP2P_LEAVE_ACK;
		this->iPktSeq	= iPktSeq;
		this->iCellID	= iCellID;
		this->iFromID	= iFromID;
	}
};
#pragma pack(pop)
