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
#include "NetPc.h"
#include "NetPcManager.h"
#include "Util.h"

P2PAgent* P2PAgentFactory::Create( const Network_IF& NIF, P2PAgentHandler* pHandler,
	unsigned int iMaxBufferSize, unsigned long iKeepConnection )
{
	WSADATA WSAData;
	if( WSAStartup( MAKEWORD(2, 2), &WSAData) != 0 )
	{
		printf( "WSAStartup failure: error=%d\r\n", WSAGetLastError() );
		return NULL;
	}

	UDPLink* pUDP			= new UDPLink;
	std::string sIP			= Util::NetIF2IP( &NIF );
	unsigned short iPort	= Util::NetIF2Port( &NIF );
	if ( !pUDP->Open(sIP, iPort, iMaxBufferSize) )
	{
		delete pUDP;
		return NULL;
	}

	NetPcManager* pMgr = new NetPcManager(&NIF, pHandler, pUDP, iKeepConnection);
	return pMgr;
}

namespace P2PUtil
{
	void SetNetworkAddress( Network_IF* pNetIF, const std::string& sIP, unsigned short iPort )
	{
		Util::SetNetworkAddress( pNetIF, sIP, iPort );
	}

	std::string Addr2Str( Network_IF* pNetIF )
	{
		return Util::Addr2Str( pNetIF );
	}
};
