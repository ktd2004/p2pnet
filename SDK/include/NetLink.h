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

//////////////////////////////////////////////////////////////////////////////////////
// NetLink 의 네트웍 정보
struct Network_IF
{
	unsigned int	iCellID;
	unsigned int	iID;		// link unique id
	unsigned long	iAddr;		// ip
	unsigned short	iPort;		// port
	Network_IF()
		: iCellID(0)
		, iID(0)
		, iAddr(0)
		, iPort(0)
	{}
	Network_IF(unsigned int _iID)
		: iCellID(0)
		, iID(_iID)
		, iAddr(0)
		, iPort(0)
	{}
};

//////////////////////////////////////////////////////////////////////////////////////
// NetLink 의 상태정보

enum eLinkST
{
	eUNLINK_ST,		// initial state
	eSYNC_ST,		// sync state
	eSYNC_ACK_ST,	// sync-ack state
	eLINK_ST,		// link state
};

struct Network_ST
{
	unsigned short	iSt;					// link state
	unsigned long	iUnreliablePktSeq;		// unreliable packet seq
	unsigned long	iReliablePktSeq;		// reliable packet seq
	unsigned long	iWishReliablePktSeq;	// wish reliable packet seq
	unsigned long	iControlPktSeq;			// control packet seq
	unsigned long	iWishControlPktSeq;		// wish control packet seq
	unsigned long	iKeepConnection;		// keep connection interval
	unsigned long	iReceivedPktTm;			// packet received time
	unsigned long	iAvgLatency;			// avg latency
	Network_ST()
		: iSt(eUNLINK_ST)
		, iUnreliablePktSeq(0)
		, iReliablePktSeq(0)
		, iWishReliablePktSeq(0)
		, iControlPktSeq(0)
		, iWishControlPktSeq(0)
		, iKeepConnection(0)
		, iReceivedPktTm(0)
		, iAvgLatency(0)
	{}
};

//////////////////////////////////////////////////////////////////////////////////////
// NetLink
struct NetLink : public Network_IF, public Network_ST
{
	virtual Network_IF&			NetIF( void ) = 0;
	virtual void				NetIF( const Network_IF& r ) = 0;
	virtual Network_ST&			NetST( void ) = 0;
	virtual void				NetST( const Network_ST& r ) = 0;
	virtual bool				Push( const char* pPkt, unsigned int iLen, bool bReliable = false ) = 0;
	virtual void				Clear( void ) = 0;
};