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

//<
#include <string>
#include <windows.h>
#include <WinSock2.h>

//<
#include "P2PPkt.h"
#include "Md5Hash.h"

namespace Util
{
	inline void SetNetworkAddress( P2PNET_PACKET_BASE* pPkt, const std::string& sIP, unsigned short iPort )
	{
		pPkt->Addr.sin_family			= AF_INET;
		pPkt->Addr.sin_port				= htons( iPort );			// network port 로 변환
		pPkt->Addr.sin_addr.S_un.S_addr = inet_addr( sIP.c_str() ); // network address 로 변환
	}

	inline void SetNetworkAddress( P2PNET_PACKET_BASE* pPkt, const Network_IF& rNetIF )
	{
		pPkt->Addr.sin_family			= AF_INET;
		pPkt->Addr.sin_port				= rNetIF.iPort;
		pPkt->Addr.sin_addr.S_un.S_addr = rNetIF.iAddr;
	}

#define shift(v,n)				(((unsigned __int64)v) << n)
#define addr2nid(addr,v)		shift(addr.sin_addr.S_un.S_addr,32) | shift(addr.sin_port,16) | shift(v,0)

	inline void SetNetworkAddress( Network_IF* pNIF, const std::string& sIP, unsigned short iPort )
	{
		pNIF->iAddr = inet_addr( sIP.c_str() );	// network address 로 변환
		pNIF->iPort = htons( iPort );				// network port 로 변환
	}

	inline std::string Addr2Str( Network_IF* pNIF )
	{
		SOCKADDR_IN addr;
		addr.sin_addr.S_un.S_addr = pNIF->iAddr;
		char szBuf[25]; sprintf_s( szBuf, sizeof(szBuf), "%s:%u", inet_ntoa(addr.sin_addr), ntohs(pNIF->iPort) );
		return szBuf;
	}

	inline std::string NetIF2IP( const Network_IF* pNIF )
	{
		SOCKADDR_IN addr;
		addr.sin_addr.S_un.S_addr = pNIF->iAddr;
		return inet_ntoa(addr.sin_addr);
	}

	inline unsigned short NetIF2Port( const Network_IF* pNIF )
	{
		return ntohs(pNIF->iPort);
	}
};