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
#include "UDPLink.h"
#include "P2PPkt.h"

//<
#include <P2PAgent.h>

class NetLinkManager : public P2PAgent
{
	friend class Pc;
protected:
	P2PAgentHandler*	m_Handler;
	UDPLink*			m_Link;
	NetLinkMap			m_NetLinkMap;
	NetLink*			m_Self;				// self link
	NetLink*			m_Relay;			// relay link
	P2PNET_PACKET_LIST	m_Received;			// received packet list
	unsigned long		m_iKeepConnection;	// alive check

public:
	NetLinkManager(P2PAgentHandler* pHandler, UDPLink* pLink, unsigned long iKeepConnection);
	virtual ~NetLinkManager();

	//< P2PAgent
	NetLinkMap&			GetHashMap( void ) { return m_NetLinkMap; }
	NetLink*			Self( const Network_IF& rNetIF );
	NetLink*			Self( void );
	NetLink*			Find( const Network_IF& rNetIF );
	NetLink*			Insert( const Network_IF& rNetIF );
	bool				Erase( const Network_IF& rNetIF );
	void				Clear( void );
	void				Process( void );
	//<
	P2PAgentHandler*	Handler( void );
	P2PAgentHandler*	Handler( P2PAgentHandler* pHandler );
	NetLink*			Relay( const Network_IF& rNetIF );
	NetLink*			Relay( void );
	//<
	virtual int			OnConnected( NetLink* pLink );
	virtual int			OnClosed( NetLink* pLink );
	virtual int			OnReceived( NetLink* pLink, const char* pPkt, unsigned int iLen );
	virtual int			OnSended( NetLink* pLink, const char* pPkt, unsigned int iLen );
	virtual int			OnError( NetLink* pLink, const char* pPkt, unsigned int iLen );
	//<
	virtual NetLink*	OnCreate( void ) = 0;
};