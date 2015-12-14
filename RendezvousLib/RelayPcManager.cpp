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
#include "RelayPc.h"
#include "RelayPcManager.h"
#include <Util.h>

//<
#include <windows.h>
#include <Mmsystem.h>

RelayPcManager::RelayPcManager( const Network_IF* pNIF, UDPLink* pLink, unsigned long iKeepConnection )
: NetLinkManager( pNIF, NULL, pLink, iKeepConnection )
{
}

RelayPcManager::~RelayPcManager()
{
}

NetLink* RelayPcManager::OnCreate()
{
	return (NetLink*)new RelayPc( this, m_Link );
}

int RelayPcManager::OnConnected( NetLink* pLink )
{
	verbose( __FUNCTION__"\n" );
	P2P_JOIN* p = NULL;
	for ( NetLinkMap::iterator it = m_NetLinkMap.begin(); it != m_NetLinkMap.end(); ++it )
	{
		RelayPc* pc = (RelayPc*)it->second;
		if ( pLink != (NetLink*)pc )
		{	// 이미 조인한 모든 유저에게 알린다.
			verbose( "send join %u to others %s\n", pLink->NetIF().iNID, Util::Addr2Str(&pc->NetIF()).c_str() );
			p = new P2P_JOIN( 0, m_Self.iNID, pLink->NetIF() );
			pc->PushControl( p );
		}

		// 현재 조인한 모든 유저 정보를 알린다.
		verbose( "send members %u to self %s\n", pc->NetIF().iNID, Util::Addr2Str(&pc->NetIF()).c_str() );			
		p = new P2P_JOIN( 0, m_Self.iNID, pc->NetIF() );
		((RelayPc*)pLink)->PushControl( p );
	}

	return 0;
}

int RelayPcManager::OnClosed( NetLink* pLink )
{
	verbose( __FUNCTION__"\n" );
	// relay 상태인 NetPc들은 서로의 상태를 알수 없기 때문에 랑데뷰서버에서 상태정보를 갱신
	P2P_LEAVE* p = NULL;
	for ( NetLinkMap::iterator it = m_NetLinkMap.begin(); it != m_NetLinkMap.end(); ++it )
	{
		RelayPc* pc = (RelayPc*)it->second;
		if ( pLink != (NetLink*)pc )
		{	// 이미 조인한 모든 유저에게 알린다.
			verbose( "send leave %u to others %s\n", pLink->NetIF().iNID, Util::Addr2Str(&pc->NetIF()).c_str() );
			p = new P2P_LEAVE( 0, m_Self.iNID, pLink->NetIF() );
			pc->PushControl( p );
		}
	}
	return 0;
}

int RelayPcManager::OnReceived( NetLink* pLink, const char* pPkt, unsigned int iLen )
{
	// 패킷을 릴레이 하면 된다.
	P2PNET_PACKET_BASE* p = (P2PNET_PACKET_BASE*)pPkt;

	NetLinkMap::iterator it = m_NetLinkMap.find( p->iToID );
	if ( it != m_NetLinkMap.end() )
	{
		RelayPc* pc = (RelayPc*)it->second;
		// LYJ 여기도 고쳐야 된다.
		// Pc::Process() 에서 패킷을 전송할때 패킷의 전송 tick을 변경한다.
		// 릴레이에서는 그냥 전송해야 된다.
		Util::SetNetworkAddress( p, pc->NetIF() );
		verbose( "received relay from %u to %u(%s)\n", p->iFromID, p->iToID, Util::Addr2Str(&pc->NetIF()).c_str() );
		m_Link->Send((const char*)&p->iLen, p->iLen, &p->Addr);
		return 0;
	}

	delete[] pPkt;
	return 0;
}

int RelayPcManager::OnSended( NetLink* pLink, const char* pPkt, unsigned int iLen )
{
	return 0;
}

int RelayPcManager::OnError( NetLink* pLink, const char* pPkt, unsigned int iLen )
{
	return 0;
}