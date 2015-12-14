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
#include "UDPLink.h"

UDPLink::UDPLink()
{
	m_hSocket			= INVALID_SOCKET;
	m_iMaxBufferSize	= 4096;
	m_Wait.tv_sec		= 0;
	m_Wait.tv_usec		= 0;
}

UDPLink::UDPLink( const UDPLink &r )
{
	m_hSocket			= r.m_hSocket;
	m_iMaxBufferSize	= r.m_iMaxBufferSize;
	m_Wait				= r.m_Wait;
}

UDPLink::~UDPLink()
{
	Close();
}

bool UDPLink::Open( const std::string& pszIP, unsigned short iPort, unsigned int iMaxBufferSize )
{
	if( m_hSocket == INVALID_SOCKET )
	{
		m_iMaxBufferSize = iMaxBufferSize;
		m_hSocket = socket( AF_INET , SOCK_DGRAM , IPPROTO_UDP );
		if( m_hSocket == INVALID_SOCKET ) return false;		

		struct sockaddr_in addr;
		memset( &addr, 0, sizeof(addr) );
		addr.sin_family      = AF_INET;
		addr.sin_port        = htons(iPort);

		if ( pszIP.empty() )
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		else
			addr.sin_addr.S_un.S_addr = inet_addr(pszIP.c_str());
		
		int iResult = bind(m_hSocket, (struct sockaddr *)&addr, sizeof(addr) );
		if ( iResult == SOCKET_ERROR || iResult == -1 )
		{
			DWORD dwErrorCode = WSAGetLastError();
			printf("error: %s,%d Open() socket Error = %d\n", __FILE__,__LINE__, dwErrorCode );
			closesocket( m_hSocket );
			m_hSocket = INVALID_SOCKET;
			return false;
		}

		int iOn = 1;
		setsockopt( m_hSocket , SOL_SOCKET , SO_REUSEADDR , (char *)&iOn , sizeof(int) );
		int iSize = 262142;
		setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&iSize, sizeof(int) );
		iSize = 262142;
		setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&iSize, sizeof(int) );

		unsigned long lCmd = 1;
		ioctlsocket( m_hSocket, FIONBIO, &lCmd );
	}

	return true;
}

void UDPLink::Close()
{
	if ( m_hSocket != INVALID_SOCKET )
	{
		shutdown( m_hSocket , SD_SEND | SD_RECEIVE );
		closesocket( m_hSocket );
		m_hSocket = INVALID_SOCKET;
	}
}

bool UDPLink::IsOpen()
{
	return m_hSocket != INVALID_SOCKET ? true : false;
}

bool UDPLink::GetSockInfo( std::string& sIP, unsigned short& iPort )
{
	struct sockaddr_in addr;
	int addrsize = sizeof(addr);
	memset((void *)&addr, 0, addrsize);
    int nResult = getsockname(m_hSocket, (sockaddr*)&addr, &addrsize);
    if(nResult == SOCKET_ERROR)
		return false;

	sIP		= inet_ntoa(addr.sin_addr);
	iPort	= ntohs(addr.sin_port);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

int UDPLink::Send( const char* pszBuf, unsigned int iLen, SOCKADDR_IN* to )
{
	FD_ZERO( &m_wFdSet );
	FD_SET( m_hSocket, &m_wFdSet);

	int iResult, iPos = 0;
	iResult = select( 0, NULL, &m_wFdSet, NULL, &m_Wait );
	if ( iResult == 0 )
	{
		return 0;
	}

retry_send:

	iResult = sendto(
				m_hSocket,
				pszBuf + iPos,
				iLen,
				0,
				(SOCKADDR *)to,
				sizeof(SOCKADDR) );

	if ( iResult == SOCKET_ERROR )
	{
		DWORD dwErrorCode = WSAGetLastError();
		if ( dwErrorCode != ERROR_IO_PENDING && dwErrorCode != WSAEWOULDBLOCK )
		{
			printf("error: %s,%d sendto %d\n", __FILE__,__LINE__, dwErrorCode );
			return -1;
		}
	}

	if ( (unsigned int)iResult < iLen )
	{
		iLen -= iResult;
		iPos += iResult;
		goto retry_send;
	}

	return iResult;
}


bool UDPLink::IsReceived( unsigned long iWait )
{
	m_Wait.tv_usec = iWait * 1000;
	FD_ZERO( &m_rFdSet );
	FD_SET( m_hSocket, &m_rFdSet);

	int iResult;
	iResult = select( 0, &m_rFdSet, NULL, NULL, &m_Wait );
	if ( iResult == 0 )
	{
		return false;
	}

	return true;
}

int UDPLink::RecvFrom( char* pszBuf, SOCKADDR_IN* from )
{
	int iFromLen = sizeof( SOCKADDR );
	int iResult = recvfrom( m_hSocket,
					pszBuf,
					m_iMaxBufferSize,
					0,
					(SOCKADDR *)from,
					&iFromLen );

	if (iResult == SOCKET_ERROR)
	{
		DWORD dwErrorCode = WSAGetLastError();
		if ( dwErrorCode != ERROR_IO_PENDING && dwErrorCode != WSAEWOULDBLOCK && dwErrorCode != WSAECONNRESET )
		{
			printf("error: %s,%d recvfrom %d\n", __FILE__,__LINE__, dwErrorCode );
			return -1;
		}

		return 0;
	}

	return iResult;
}