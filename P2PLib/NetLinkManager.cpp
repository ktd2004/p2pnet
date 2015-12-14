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
#include "Pc.h"
#include "NetLinkManager.h"
#include "Util.h"
#include "Compile.h"

//<
#include <windows.h>
#include <Mmsystem.h>


NetLinkManager::NetLinkManager( const Network_IF* pNIF, P2PAgentHandler* pHandler, UDPLink* pLink, unsigned long iKeepConnection )
: m_Handler(pHandler)
, m_Link(pLink)
, m_Relay(NULL)
, m_iKeepConnection(iKeepConnection)
{
	m_Self = *pNIF;
}

NetLinkManager::~NetLinkManager()
{
	Clear();
	if ( m_Link )
		delete m_Link;
}

NetLink* NetLinkManager::Find( const Network_IF& rNIF )
{
	NetLinkMap::iterator it = m_NetLinkMap.find( rNIF.iNID );
	if ( it != m_NetLinkMap.end() )
	{
		return it->second;
	}

	return NULL;
}

NetLink* NetLinkManager::Connect( const Network_IF& rNIF )
{
	Pc* pLink = (Pc*)OnCreate();
	pLink->NetIF( rNIF );
	pLink->NetST( eSYNC_ST );
	verbose( "create link %u\n", pLink->iNID );
	m_NetLinkMap.insert( NetLinkMap::value_type(pLink->iNID, (NetLink*)pLink) );
	return (NetLink*)pLink;
}

bool NetLinkManager::Close( NetLink* pLink )
{
	if ( !pLink )
		return false;

	NetLinkMap::iterator it;
	it = m_NetLinkMap.find( pLink->NetIF().iNID );
	if ( it != m_NetLinkMap.end() )
	{
		delete it->second;
		m_NetLinkMap.erase( it );
		return true;
	}

	return false;
}

void NetLinkManager::Clear()
{
	for ( NetLinkMap::iterator it = m_NetLinkMap.begin(); it != m_NetLinkMap.end(); )
	{
		delete it->second;
		it = m_NetLinkMap.erase( it );
	}

	while ( !m_Received.empty() )
	{
		P2PNET_PACKET_BASE* p = m_Received.front(); m_Received.pop_front();
		delete [] p;
	}
}

void NetLinkManager::Process( unsigned long iWait )
{
	//< 패킷 수신 - MAX_PACKET_RECEIVE_TM 초 동안만 수신하고 나머지는 다음 프레임에 처리
	unsigned long iCurrTick = timeGetTime() + MAX_PACKET_RECEIVE_TM;
	while ( m_Link->IsReceived(iWait) && iCurrTick > timeGetTime() )
	{
		P2PNET_PACKET_BASE* p = (P2PNET_PACKET_BASE*)new char[m_Link->MaxBufferSize()];
		if ( m_Link->RecvFrom( (char*)&p->iLen, &p->Addr ) <= 0 )
		{
			delete p;
			break;
		}

		m_Received.push_back( p );
	}

	Pc* pLink = NULL;
	NetLinkMap::iterator it;

	// 수신 패킷 처리
	// 수신패킷 처리
	while ( !m_Received.empty() )
	{
		pLink = NULL;
		P2PNET_PACKET_BASE* pPkt = m_Received.front(); m_Received.pop_front();
		it = m_NetLinkMap.find( pPkt->iFromID );
		if ( it != m_NetLinkMap.end() )
		{
			pLink = (Pc*)it->second;
		}
		else if ( pPkt->iPackID == eP2P_SYNC )
		{
			// 등록되지 않은 사용자는 Pc 를 생성한다.
			Network_IF nif;
			nif.iCellID = pPkt->iCellID;
			nif.iNID	= pPkt->iFromID;
			nif.iAddr	= pPkt->Addr.sin_addr.S_un.S_addr;
			nif.iPort	= pPkt->Addr.sin_port;
			pLink		= (Pc*)Connect( nif );
			pLink->OnSync( pPkt );
			continue;
		}
		else if ( pPkt->iPackID == eP2P_JOIN )
		{
			P2P_JOIN_ACK p(pPkt->iPktSeq, pPkt->iCellID, Self().iNID, pPkt->iTransTick);
			m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);
			delete[] pPkt;
			continue;
		}
		else if ( pPkt->iPackID == eP2P_LEAVE )
		{
			P2P_LEAVE_ACK p(pPkt->iPktSeq, pPkt->iCellID, Self().iNID, pPkt->iTransTick);
			m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);
			delete[] pPkt;
			continue;
		}
		else
		{
			verbose( "pkt id %d, from %u\n", pPkt->iPackID, pPkt->iFromID );
			delete[] pPkt;
			continue;
		}

		pLink->OnReceived( pPkt );
	}

	// 패킷 전송 및 연결유지
	for ( it = m_NetLinkMap.begin(); it != m_NetLinkMap.end(); )
	{
		pLink = (Pc*)it->second;
		switch ( pLink->m_iSt )
		{
		case eSYNC_ST :
			pLink->Sync();
			break;
		case eLINK_ST :
			if ( pLink->KeepConnection() )
			{
				it = m_NetLinkMap.erase( it );
				delete pLink;
				continue;
			}
			break;
		}

		pLink->Process();
		++it;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

P2PAgentHandler* NetLinkManager::Handler( void )
{
	return m_Handler;
}

P2PAgentHandler* NetLinkManager::Handler( P2PAgentHandler* pHandler )
{
	m_Handler = pHandler;
	return m_Handler;
}

NetLink* NetLinkManager::Relay( const Network_IF& rNIF )
{
	m_Relay = Connect( rNIF );
	verbose( "create relay %u, public %s\n", m_Relay->NetIF().iNID, Util::Addr2Str(&m_Relay->NetIF()).c_str() );
	return m_Relay;
}

NetLink* NetLinkManager::Relay( void )
{
	return m_Relay;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int	NetLinkManager::OnConnected( NetLink* pLink )
{
	if ( m_Handler ) m_Handler->OnConnected( pLink );
	return 0;
}

int	NetLinkManager::OnClosed( NetLink* pLink )
{
	if ( m_Handler ) m_Handler->OnClosed( pLink );
	return 0;
}

int	NetLinkManager::OnReceived( NetLink* pLink, const char* pPkt, unsigned int iLen )
{
	if ( m_Handler ) m_Handler->OnReceived( pLink, pPkt, iLen );
	return 0;
}

int	NetLinkManager::OnSended( NetLink* pLink, const char* pPkt, unsigned int iLen )
{
	if ( m_Handler ) m_Handler->OnSended( pLink, pPkt, iLen );
	return 0;
}

int	NetLinkManager::OnError( NetLink* pLink, const char* pPkt, unsigned int iLen )
{
	if ( m_Handler ) m_Handler->OnSended( pLink, pPkt, iLen );
	return 0;
}