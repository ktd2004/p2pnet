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

#include <string>
#include <WinSock2.h>

typedef unsigned int		TNID;

//< link network information
struct Network_IF
{
	unsigned int		iCellID;	// cell id
	TNID				iNID;		// network id
	unsigned long		iAddr;		// ip
	unsigned short		iPort;		// port
	Network_IF()
		: iCellID(0)
		, iNID(0)
		, iAddr(0)
		, iPort(0)
	{}
	Network_IF( TNID ID )
		: iCellID(0)
		, iNID(ID)
		, iAddr(0)
		, iPort(0)
	{}
	Network_IF( TNID ID, const std::string& sIP, unsigned short iPort )
		: iCellID(0)
		, iNID(ID)
	{
		this->iAddr = inet_addr( sIP.c_str() );		// network address 로 변환
		this->iPort = htons( iPort );				// network port 로 변
	}
};

//< link status
enum eLinkST
{
	eUNLINK_ST,		// initial state
	eSYNC_ST,		// sync state
	eSYNC_ACK_ST,	// sync-ack state
	eLINK_ST,		// link state
};


//< link class
struct NetLink : public Network_IF
{
	virtual Network_IF&			NetIF( void ) = 0;
	virtual void				NetIF( const Network_IF& r ) = 0;
	virtual eLinkST				NetST( void ) = 0;
	virtual void				NetST( const eLinkST r ) = 0;
	virtual bool				Push( const char* pPkt, unsigned int iLen, bool bReliable = false ) = 0;
	virtual void				Clear( void ) = 0;
};