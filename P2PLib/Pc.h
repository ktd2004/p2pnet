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

#include <NetLink.h>

#include "P2PPkt.h"
#include "UDPLink.h"
#include "Compile.h"

class Pc : public NetLink
{
public:
	//<
	NetLinkManager*		m_NetLinkManager;
	UDPLink*			m_Link;						// udp socket
	P2PNET_PACKET_LIST	m_Reliable;					// send reliable packet list
	P2PNET_PACKET_LIST	m_Reserved;					// send unreliable packet
	P2PNET_PACKET_LIST	m_WaitReliable;				// wait reliable packet
	unsigned long		m_Latency[MAX_LATENCY];		// latency
	unsigned long		m_iSyncTm;					// sync time
	unsigned short		m_iSyncCnt;					// sync count

public:
	Pc();
	Pc(NetLinkManager* pNetLinkMgr, UDPLink* pLink);
	virtual ~Pc();

public:
	//< NetLink
	Network_IF&		NetIF( void );
	void			NetIF( const Network_IF& r );
	Network_ST&		NetST( void );
	void			NetST( const Network_ST& r );
	bool			Push( const char* pPkt, unsigned int iLen, bool bReliable = false );
	virtual void	Process( void );
	virtual void	Clear( void );
	//<
	void			Sync( void );
	void			KeepConnection( void );
	bool			Push( P2PNET_PACKET_BASE* pPkt, bool bReliable );
	virtual int		OnReceived( P2PNET_PACKET_BASE* pPkt );
	void			Latency( unsigned long iLatency );
	unsigned long	Latency( void );
	//<
	unsigned long	GetAverageLatency( NetLink* pLink );
	static int		SortLatency( const void *arg1, const void *arg2 );
};