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

#include <P2PAgent.h>

struct RendezvousAgentFactory
{
	static P2PAgent*	Create( const std::string& sIP, unsigned short iPort,
							unsigned int iMaxBufferSize = 4096, unsigned long iKeepConnection = 1000 );
	static P2PAgent*	Join( P2PAgentHandler* pHdr, const std::string& sIP, unsigned short iPort, 
							const std::string& sRendezvousIP, unsigned short iRendezvousPort,
							unsigned int iMaxBufferSize = 4096, unsigned long iKeepConnection = 1000 );
};