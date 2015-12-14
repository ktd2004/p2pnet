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

RelayPc::RelayPc(const RelayPc& r)
{
	this->m_NetLinkManager		= r.m_NetLinkManager;
	this->m_Link				= r.m_Link;
	this->iCellID				= r.iCellID;
	this->iID					= r.iID;
	this->iAddr					= r.iAddr;
	this->iPort					= r.iPort;
	this->iSt					= r.iSt;
	this->iUnreliablePktSeq		= r.iUnreliablePktSeq;
	this->iReliablePktSeq		= r.iReliablePktSeq;
	this->iWishReliablePktSeq	= r.iWishReliablePktSeq;
	this->iControlPktSeq		= r.iControlPktSeq;
	this->iWishControlPktSeq	= r.iWishControlPktSeq;
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
			p->iTransTick = iTick;
			if ( m_Link->Send((const char*)&p->iLen, p->iLen, &p->Addr) == -1 )
			{
				m_NetLinkManager->OnError( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
				break;
			}
			p->iTransTick = iTick + iAvgLatency;
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
	pPkt->iFromID = m_NetLinkManager->Self()->NetIF().iID;
	pPkt->iPktSeq = iControlPktSeq++;
	m_Control.push_back( pPkt );
}

int RelayPc::OnReceived( P2PNET_PACKET_BASE* pPkt )
{
	if ( Pc::OnReceived(pPkt) )
	{
		delete[] pPkt;
		return 1;
	}

	switch (pPkt->iPackID)
	{
	case eP2P_JOIN_ACK				:
	case eP2P_LEAVE_ACK				:
		{
			Latency(  NetST().iReceivedPktTm - pPkt->iTransTick );
			verbose( "recv control ack %d\n", pPkt->iPktSeq );
			for ( P2PNET_PACKET_LIST::iterator it = m_Control.begin(); it != m_Control.end(); it++ )
			{
				P2PNET_PACKET_BASE* p = *it;
				if ( p->iPktSeq == pPkt->iPktSeq )
				{
					verbose( "erase control ack %d\n", pPkt->iPktSeq );
					m_Control.erase( it );
					m_NetLinkManager->OnSended( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
					delete[] p;
					break;
				}
			}
		}
		break;
	default :
		m_NetLinkManager->OnReceived( this, (const char*)pPkt, pPkt->iLen );
		return 1;
	}
	
	delete[] pPkt;
	return 1;
}