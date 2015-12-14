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
#include "stdafx.h"

//<
#include "NetPcManager.h"
#include "NetPc.h"
#include "Util.h"

//<
#include <P2PAgent.h>

//< 위로 올리지 말것
#include <Mmsystem.h>

NetPc::NetPc()
{

}

NetPc::NetPc(NetPcManager* pNetPcMgr, UDPLink* pLink)
: Pc(pNetPcMgr, pLink)
{
}

NetPc::~NetPc()
{
}

int NetPc::OnJoin( P2PNET_PACKET_BASE* pPkt )
{
	m_iReceivedPktTm = timeGetTime();

//	verbose( "recv join %u\n", pPkt->iFromID  );
	P2P_JOIN_ACK p(pPkt->iPktSeq, pPkt->iCellID, m_NetLinkManager->Self().iNID, pPkt->iTransTick);
	p.iTransTick = pPkt->iTransTick;
	m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);
	Network_IF& nif = ((P2P_JOIN*)pPkt)->NIF;

	if ( m_iWishControlPktSeq == pPkt->iPktSeq )
	{
		m_iWishControlPktSeq++;
		if ( m_NetLinkManager->Self().iNID == nif.iNID )
		{
			verbose( "join self %u, public %s\n", nif.iNID, Util::Addr2Str(&nif).c_str() );
		}
		else
		{
			verbose( "join peer %u public %s\n", nif.iNID, Util::Addr2Str(&nif).c_str() );
			m_NetLinkManager->Connect( nif );
		}
	}

	delete[] pPkt;

	return 1;
}

int NetPc::OnLeave( P2PNET_PACKET_BASE* pPkt )
{
	m_iReceivedPktTm = timeGetTime();

//	verbose( "recv join %u\n", pPkt->iFromID  );
	P2P_LEAVE_ACK p(pPkt->iPktSeq, pPkt->iCellID,  m_NetLinkManager->Self().iNID, pPkt->iTransTick);
	p.iTransTick = pPkt->iTransTick;
	m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);
	Network_IF& nif = ((P2P_JOIN*)pPkt)->NIF;

	if ( m_iWishControlPktSeq == pPkt->iPktSeq )
	{
		m_iWishControlPktSeq++;
		if ( m_NetLinkManager->Self().iNID == nif.iNID )
		{
			verbose( "leave self %u, public %s\n", nif.iNID, Util::Addr2Str(&nif).c_str() );
		}
		else
		{
			verbose( "leave peer %u public %s\n", nif.iNID, Util::Addr2Str(&nif).c_str() );
			NetLink* pLink = m_NetLinkManager->Find( nif );
			m_NetLinkManager->Close( pLink );
		}
	}

	delete[] pPkt;

	return 1;
}

int NetPc::OnReliableAck( P2PNET_PACKET_BASE* pPkt )
{
	m_iReceivedPktTm = timeGetTime();

	Latency( m_iReceivedPktTm - pPkt->iTransTick );
	verbose( "recv reliable ack %lu\n", pPkt->iPktSeq );
	for ( P2PNET_PACKET_LIST::iterator it = m_Reliable.begin(); it != m_Reliable.end(); it++ )
	{
		P2PNET_PACKET_BASE* p = *it;
		if ( p->iPktSeq == pPkt->iPktSeq )
		{
			verbose( "erase reliable ack %lu\n", pPkt->iPktSeq );
			m_Reliable.erase( it );
			m_NetLinkManager->OnSended( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
			delete[] p;
			break;
		}
	}

	delete[] pPkt;

	return 1;
}

int NetPc::OnReliableSendTo( P2PNET_PACKET_BASE* pPkt )
{
	m_iReceivedPktTm = timeGetTime();

	verbose( "recv reliable pkt %lu, wish %lu\n", pPkt->iPktSeq, m_iWishReliablePktSeq );
//	verbose( "from %u to %u\n", iNID, m_NetLinkManager->Self().iNID );
	P2P_RELIABLE_ACK p(pPkt->iPktSeq, pPkt->iCellID, m_NetLinkManager->Self().iNID, iNID, pPkt->iTransTick);
	m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);

	// 수신 받은 패킷 순서가 올바른지 검증
	if ( m_iWishReliablePktSeq == pPkt->iPktSeq )
	{
		verbose( "find reliable pkt %lu\n", pPkt->iPktSeq );
		m_iWishReliablePktSeq++;
		m_NetLinkManager->OnReceived( this, PKT_DATA_POS(pPkt), PKT_DATA_LEN(pPkt) );

		// 혹시 순서가 바뀐 패킷이 있는지 확인
		for ( P2PNET_PACKET_LIST::iterator it = m_WaitReliable.begin(); it != m_WaitReliable.end(); )
		{
			P2PNET_PACKET_BASE* p = *it;
			if ( m_iWishReliablePktSeq == p->iPktSeq )
			{
				verbose( "find reliable pkt %lu\n", pPkt->iPktSeq );
				m_iWishReliablePktSeq++;
				m_NetLinkManager->OnReceived( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
				it = m_WaitReliable.erase(it);
				delete[] p;
			}
			else if ( m_iWishReliablePktSeq > p->iPktSeq )
			{
				it = m_WaitReliable.erase(it);
				delete[] p;
			}
			else
			{
				it++;
			}
		}
	}
	else if ( m_iWishReliablePktSeq < pPkt->iPktSeq )
	{
		verbose( "wait reliable pkt %lu\n", pPkt->iPktSeq );
		m_WaitReliable.push_back( pPkt );
		return 0;
	}

	delete[] pPkt;

	return 1;
}


int NetPc::OnReceived( P2PNET_PACKET_BASE* pPkt )
{
	switch (pPkt->iPackID)
	{
	case eP2P_SYNC :
		OnSync( pPkt );
		break;
	case eP2P_SYNC_ACK :
		OnSyncAck( pPkt );
		break;
	case eP2P_KEEPING_CONNECTION :
		OnKeepConnection( pPkt );
		break;
	case eP2P_JOIN :
		OnJoin( pPkt );
		break;
	case eP2P_LEAVE :
		OnLeave( pPkt );
		break;
	case eP2P_RELIABLE_ACK :
		OnReliableAck( pPkt );
		break;
	case eP2P_RELIABLE_SEND_TO :
		OnReliableSendTo( pPkt );
		break;
	default :
		m_iReceivedPktTm = timeGetTime();
		m_NetLinkManager->OnReceived( this, PKT_DATA_POS(pPkt), PKT_DATA_LEN(pPkt) );
		delete[] pPkt;
		break;
	}

	return 1;
}