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
#include "RelayPcManager.h"
#include "RelayPc.h"
#include "Util.h"

//<
#include <P2PAgent.h>

//< 위로 올리지 말것
#include <Mmsystem.h>

RelayPc::RelayPc()
{
}

RelayPc::RelayPc(RelayPcManager* pRelayPcMgr, UDPLink* pLink)
: Pc(pRelayPcMgr, pLink)
{
}


RelayPc::~RelayPc()
{
}

void RelayPc::Process( void )
{
	Pc::Process();
	// control packet
	P2PNET_PACKET_LIST::iterator it;
	for ( it = m_Control.begin(); it != m_Control.end(); it++ )
	{
		P2PNET_PACKET_BASE* p = *it;
		unsigned long iTick = timeGetTime();
		if ( p->iTransTick <= iTick )
		{
//			verbose( "send control %s\n", Util::Addr2Str(&NetIF()).c_str() );
			p->iTransTick = iTick;
			if ( m_Link->Send((const char*)&p->iLen, p->iLen, &p->Addr) == -1 )
			{
				m_NetLinkManager->OnError( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
				break;
			}
			p->iTransTick = iTick + m_iAvgLatency;
		}
	}
}

void RelayPc::Clear( void )
{
	Pc::Clear();
	while ( !m_Control.empty() )
	{
		P2PNET_PACKET_BASE* p = m_Control.front(); m_Control.pop_front();
		delete p;
	}
}

void RelayPc::PushControl( P2PNET_PACKET_BASE* pPkt )
{
	Util::SetNetworkAddress( pPkt, NetIF() );
//	pPkt->iFromID = iNID;
	pPkt->iPktSeq = m_iControlPktSeq++;
	m_Control.push_back( pPkt );
}

int RelayPc::OnControlAck( P2PNET_PACKET_BASE* pPkt )
{
	m_iReceivedPktTm = timeGetTime();

	Latency(  m_iReceivedPktTm - pPkt->iTransTick );
	verbose( "recv control ack %lu\n", pPkt->iPktSeq );
	for ( P2PNET_PACKET_LIST::iterator it = m_Control.begin(); it != m_Control.end(); it++ )
	{
		P2PNET_PACKET_BASE* p = *it;
		if ( p->iPktSeq == pPkt->iPktSeq )
		{
			verbose( "erase control ack %lu\n", pPkt->iPktSeq );
			m_Control.erase( it );
			m_NetLinkManager->OnSended( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
			delete[] p;
			break;
		}
	}

	delete[] pPkt;

	return 1;
}

int RelayPc::OnReceived( P2PNET_PACKET_BASE* pPkt )
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
	case eP2P_JOIN_ACK :
	case eP2P_LEAVE_ACK :
		OnControlAck( pPkt );
		break;
	default :
		m_iReceivedPktTm = timeGetTime();
		m_NetLinkManager->OnReceived( this, (const char*)pPkt, pPkt->iLen );
		return 1;
	}
	
	return 1;
}