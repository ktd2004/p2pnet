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
#include <P2PEngine.h>

#include <Mmsystem.h>


struct NetHandler : public P2PAgentHandler
{
	int	OnConnected( NetLink* pLink )
	{
		printf( __FUNCTION__"\n" );
		return 0;
	}
	int	OnClosed( NetLink* pLink )
	{
		printf( __FUNCTION__"\n" );
		return 0;
	}
	int	OnReceived( NetLink* pLink, const char* pPkt, unsigned int iLen )
	{
		printf( __FUNCTION__": %d\n", iLen );
		return 0;
	}
	int	OnSended( NetLink* pLink, const char* pPkt, unsigned int iLen )
	{
		printf( __FUNCTION__":%d\n", iLen );
		return 0;
	}
	int	OnError( NetLink* pLink, const char* pPkt, unsigned int iLen )
	{
		printf( __FUNCTION__":%d\n", iLen );
		return 0;
	}
};

#include <Buffer.h>

int _tmain(int argc, _TCHAR* argv[])
{
/*
	Buffer	buf;

	buf << "this is test packet";
	printf( "cap = %d, size=%d, length = %d, space = %d\n", buf.capacity(), buf.size(), buf.length(), buf.space() );
	std::string s;

	buf >> s;
	printf( "%s\n", s.c_str() );

	printf( "cap = %d, size=%d, length = %d, space = %d\n", buf.capacity(), buf.size(), buf.length(), buf.space() );
	buf.crunch();
	printf( "cap = %d, size=%d, length = %d, space = %d\n", buf.capacity(), buf.size(), buf.length(), buf.space() );

	for ( int i = 0; i < 10; i++ )
	{
		buf << i;
	}
	printf( "cap = %d, size=%d, length = %d, space = %d\n", buf.capacity(), buf.size(), buf.length(), buf.space() );

	for ( int i = 0; i < 10; i++ )
	{
		int j;
		buf >> j;
		printf( "j = %d\n", j );
	}
	printf( "cap = %d, size=%d, length = %d, space = %d\n", buf.capacity(), buf.size(), buf.length(), buf.space() );
	buf.reset();
	printf( "cap = %d, size=%d, length = %d, space = %d\n", buf.capacity(), buf.size(), buf.length(), buf.space() );
	
	return 0;
*/
	if ( argc == 1 )
		return 0;

	NetHandler	Hdr;
	P2PAgent* pAgent = NULL;
	
	if ( strcmp(argv[1], "server") == 0 )
	{
		// self
		pAgent = P2PAgentFactory::Create( &Hdr, "127.0.0.1", 25533 );
		NetLink* pSelf = pAgent->Self();
		printf( "self %u, %s\n", pSelf->NetIF().iID, P2PUtil::Addr2Str(&pSelf->NetIF()).c_str() );

		while (1)
		{
			pAgent->Process();
			Sleep(100);
		}
	}
	else
	{
		// self id
		pAgent = P2PAgentFactory::Create( &Hdr, "127.0.0.1", 25534 );
		NetLink* pSelf = pAgent->Self();
		printf( "self %u, %s\n", pSelf->NetIF().iID, P2PUtil::Addr2Str(&pSelf->NetIF()).c_str() );

		// peer
		Network_IF NetIF;
		P2PUtil::SetNetworkAddress( &NetIF, "127.0.0.1", 25533 );
		printf( "peer %u, %s\n", NetIF.iID, P2PUtil::Addr2Str(&NetIF).c_str() );
		NetLink* pLink = pAgent->Insert( NetIF );

		unsigned long iSendTick = 0;
		while (1)
		{
			if ( pLink && pLink->NetST().iSt == eLINK_ST && iSendTick <= timeGetTime() )
			{
				char szBuf[1024];
				int iLen = sprintf_s(szBuf, 1024, "this is test" );
				pLink->Push( szBuf, iLen+1, true );
				iSendTick = timeGetTime() + 1000;

			}
			pAgent->Process();
			Sleep(100);
		}
	}

	delete pAgent;

	return 0;
}