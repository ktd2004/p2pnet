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

RelayPcManager::RelayPcManager( UDPLink* pLink, unsigned long iKeepConnection )
: NetLinkManager( NULL, pLink, iKeepConnection )
{
	std::string sIP;
	unsigned short iPort;
	pLink->GetSockInfo( sIP, iPort );

	m_Self = OnCreate();
	Network_IF& rNetIF = m_Self->NetIF();
	Util::SetNetworkAddress( &rNetIF, sIP, iPort );
}

RelayPcManager::~RelayPcManager()
{
}

NetLink* RelayPcManager::OnCreate( void )
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
			verbose( "send join %u to others %s\n", pLink->NetIF().iID, Util::Addr2Str(&pc->NetIF()).c_str() );
			p = new P2P_JOIN( 0, m_Self->NetIF().iID, pLink->NetIF() );
			pc->PushControl( p );
		}

		// 현재 조인한 모든 유저 정보를 알린다.
		verbose( "send members %u to self %s\n", pc->NetIF().iID, Util::Addr2Str(&pc->NetIF()).c_str() );			
		p = new P2P_JOIN( 0, m_Self->NetIF().iID, pc->NetIF() );
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
			verbose( "send leave %u to others %s\n", pLink->NetIF().iID, Util::Addr2Str(&pc->NetIF()).c_str() );
			p = new P2P_LEAVE( 0, m_Self->NetIF().iID, pLink->NetIF() );
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
		// 패킷을 릴레이하기 위해서 그냥 넣는다.
		verbose( "received relay from %u to %u\n", p->iFromID, p->iToID );
		RelayPc* pc = (RelayPc*)it->second;
		Util::SetNetworkAddress( p, pc->NetIF() );
		pc->m_Reserved.push_back( p );
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