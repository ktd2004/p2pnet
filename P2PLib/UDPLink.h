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
#include <windows.h>
#include <WinSock2.h>
#include <string>

//< udp socket
class UDPLink
{
	friend class NetPc;
	friend class NetPcManager;
	friend class NetLinkManager;
private:
	SOCKET			m_hSocket;				// socket
	unsigned int 	m_iMaxBufferSize;		// max packet buffer size
	struct timeval  m_Wait;					// wait for event
	fd_set			m_wFdSet;				// write
	fd_set			m_rFdSet;				// read

public:
	UDPLink();
	UDPLink( const UDPLink &r );
	virtual	~UDPLink();

	//<
	bool			Open( const std::string& sIP, unsigned short iPort, unsigned int iMaxBufferSize = 4096 );
	void			Close( void );
	bool			IsOpen( void );

	//<
	SOCKET			Socket( void ) { return m_hSocket; }
	void			Socket( SOCKET hSocket ) { m_hSocket = hSocket; }
	bool			GetSockInfo( std::string& sIP, unsigned short& iPort );
	void			MaxBufferSize( unsigned int  iMaxBufferSize ) { m_iMaxBufferSize = iMaxBufferSize; }
	unsigned int	MaxBufferSize( void ) { return m_iMaxBufferSize; }
	int				Send( const char* pszBuf, unsigned int iLen, SOCKADDR_IN* to );
	bool			IsReceived( unsigned long iWait );			
	int				RecvFrom( char* pszBuf, SOCKADDR_IN* from );
};