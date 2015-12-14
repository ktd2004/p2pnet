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


struct NetHandler : public P2PAgentHandler
{
	int	OnConnected( NetLink* pLink )
	{
		printf( __FUNCTION__": peer %u, %s\n", pLink->NetIF().iID, P2PUtil::Addr2Str(&pLink->NetIF()).c_str() );
		return 0;
	}
	int	OnClosed( NetLink* pLink )
	{
		printf( __FUNCTION__": peer %u, %s\n", pLink->NetIF().iID, P2PUtil::Addr2Str(&pLink->NetIF()).c_str() );
		return 0;
	}
	int	OnReceived( NetLink* pLink, const char* pPkt, unsigned int iLen )
	{
		return 0;
	}
	int	OnSended( NetLink* pLink, const char* pPkt, unsigned int iLen )
	{
		printf( __FUNCTION__"\n" );
		return 0;
	}
	int	OnError( NetLink* pLink, const char* pPkt, unsigned int iLen )
	{
		printf( __FUNCTION__"\n" );
		return 0;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc == 1 )
		return 0;

	P2PAgent* pAgent = NULL;
	NetHandler Hdr;

	if ( strcmp(argv[1], "server") == 0 )
	{
		pAgent = RendezvousAgentFactory::Create( "127.0.0.1", 65522 );
		NetLink* pSelf = pAgent->Self();
		printf( "self %u, %s\n", pSelf->NetIF().iID, P2PUtil::Addr2Str(&pSelf->NetIF()).c_str() );
		if ( !pAgent ) return 0;

		while ( 1 )
		{
			pAgent->Process();
			Sleep(100);
		}
	}
	else
	{
		// Rendezvous
		pAgent = RendezvousAgentFactory::Join( &Hdr, "127.0.0.1", atoi(argv[1]), "127.0.0.1", 65522, 4096 );
		NetLink* pSelf = pAgent->Self();
		printf( "self %u, %s\n", pSelf->NetIF().iID, P2PUtil::Addr2Str(&pSelf->NetIF()).c_str() );
		
		unsigned long iSendTick = timeGetTime() + 1000 * 10;
		while (1)
		{
			NetLinkMap::iterator it;
			for ( it = pAgent->GetHashMap().begin(); it != pAgent->GetHashMap().end(); ++it )
			{
				if ( it->second->NetST().iSt == eLINK_ST && pAgent->Relay() != it->second && iSendTick < timeGetTime() )
				{
					char szBuf[1024];
					int iLen = sprintf_s(szBuf, 1024, "this is test" );
					it->second->Push( szBuf, iLen+1, true );
					iSendTick = timeGetTime() + 1000 * 10;
				}
			}

			pAgent->Process();
			Sleep(100);
		}

	}

	return 0;
}

