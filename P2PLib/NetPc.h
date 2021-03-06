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

#include "Pc.h"

//<
class NetPcManager;
class NetPc : public Pc
{
public:

public:
	NetPc();
	NetPc(NetPcManager* pNetPcMgr, UDPLink* pLink);
	virtual ~NetPc();

public:
	//<
	int		OnJoin( P2PNET_PACKET_BASE* pPkt );
	int		OnLeave( P2PNET_PACKET_BASE* pPkt );
	int		OnReliableAck( P2PNET_PACKET_BASE* pPkt );
	int		OnReliableSendTo( P2PNET_PACKET_BASE* pPkt );
	int		OnReceived( P2PNET_PACKET_BASE* pPkt );
};