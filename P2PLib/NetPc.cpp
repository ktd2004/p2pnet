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

NetPc::NetPc(const NetPc& r)
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

NetPc::~NetPc()
{
}

 int NetPc::OnReceived( P2PNET_PACKET_BASE* pPkt )
{
	if ( Pc::OnReceived(pPkt) )
	{
		delete[] pPkt;
		return 1;
	}

	switch (pPkt->iPackID)
	{
	case eP2P_RELIABLE_ACK			:
		{
			Latency(  NetST().iReceivedPktTm - pPkt->iTransTick );
			verbose( "recv reliable ack %lu\n", pPkt->iPktSeq );
			for ( P2PNET_PACKET_LIST::iterator it = m_Reliable.begin(); it != m_Reliable.end(); it++ )
			{
				P2PNET_PACKET_BASE* p = *it;
				if ( p->iPktSeq == pPkt->iPktSeq )
				{
					verbose( "erase reliable ack %d\n", pPkt->iPktSeq );
					m_Reliable.erase( it );
					m_NetLinkManager->OnSended( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
					delete[] p;
					break;
				}
			}
		}
		break;
	case eP2P_JOIN					:
		{
			//verbose( "recv join %u, self %u\n", pPkt->iFromID, m_NetLinkManager->Self()->NetIF().iID  );
			P2P_JOIN_ACK p(pPkt->iPktSeq, pPkt->iCellID, m_NetLinkManager->Self()->NetIF().iID);
			p.iTransTick = pPkt->iTransTick;
			m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);
			Network_IF& nif = ((P2P_JOIN*)pPkt)->NIF;

			if ( iWishControlPktSeq == pPkt->iPktSeq )
			{
				iWishControlPktSeq++;
				if ( m_NetLinkManager->Self()->NetIF().iID == nif.iID )
				{
					verbose( "join self %u, public %s\n", nif.iID, Util::Addr2Str(&nif).c_str() );
					m_NetLinkManager->Self(nif);
				}
				else
				{
					verbose( "join peer %u public %s\n", nif.iID, Util::Addr2Str(&nif).c_str() );
					m_NetLinkManager->Insert( nif );
				}
			}
		}
		break;
	case eP2P_LEAVE					:
		{
			//verbose( "recv join %u, self %u\n", pPkt->iFromID, m_NetLinkManager->Self()->NetIF().iID  );
			P2P_LEAVE_ACK p(pPkt->iPktSeq, pPkt->iCellID, m_NetLinkManager->Self()->NetIF().iID);
			p.iTransTick = pPkt->iTransTick;
			m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);
			Network_IF& nif = ((P2P_JOIN*)pPkt)->NIF;

			if ( iWishControlPktSeq == pPkt->iPktSeq )
			{
				iWishControlPktSeq++;
				if ( m_NetLinkManager->Self()->NetIF().iID == nif.iID )
				{
					verbose( "leave self %u, public %s\n", nif.iID, Util::Addr2Str(&nif).c_str() );
				}
				else
				{
					verbose( "leave peer %u public %s\n", nif.iID, Util::Addr2Str(&nif).c_str() );
					m_NetLinkManager->Erase( nif );
				}
			}
		}
		break;
	case eP2P_RELIABLE_SEND_TO		:
		{
			verbose( "recv reliable pkt %d, wish %d\n", pPkt->iPktSeq, iWishReliablePktSeq );
			P2P_RELIABLE_ACK p(pPkt->iPktSeq, pPkt->iCellID, m_NetLinkManager->Self()->NetIF().iID, pPkt->iFromID);
			p.iTransTick = pPkt->iTransTick;
			m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);

			// 수신 받은 패킷 순서가 올바른지 검증
			if ( iWishReliablePktSeq == pPkt->iPktSeq )
			{
				verbose( "find reliable pkt %d\n", pPkt->iPktSeq );
				iWishReliablePktSeq++;
				m_NetLinkManager->OnReceived( this, PKT_DATA_POS(pPkt), PKT_DATA_LEN(pPkt) );

				// 혹시 순서가 바뀐 패킷이 있는지 확인
				for ( P2PNET_PACKET_LIST::iterator it = m_WaitReliable.begin(); it != m_WaitReliable.end(); )
				{
					P2PNET_PACKET_BASE* p = *it;
					if ( iWishReliablePktSeq == p->iPktSeq )
					{
						verbose( "find reliable pkt %d\n", pPkt->iPktSeq );
						iWishReliablePktSeq++;
						m_NetLinkManager->OnReceived( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
						it = m_WaitReliable.erase(it);
						delete[] p;
					}
					else if ( iWishReliablePktSeq > p->iPktSeq )
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
			else if ( iWishReliablePktSeq < pPkt->iPktSeq )
			{
				verbose( "wait reliable pkt %d\n", pPkt->iPktSeq );
				m_WaitReliable.push_back( pPkt );
				return 1;
			}
		}
		break;
	default :
		m_NetLinkManager->OnReceived( this, PKT_DATA_POS(pPkt), PKT_DATA_LEN(pPkt) );
		break;
	}
	
	delete[] pPkt;
	return 1;
}