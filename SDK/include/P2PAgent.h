#pragma once

//<
#include <NetLink.h>

//<
#include <hash_map>
typedef stdext::hash_map<TNID, NetLink*>	NetLinkMap;

//< p2p event handler
struct P2PAgentHandler
{
	virtual int				OnConnected( NetLink* pLink ) = 0;
	virtual int				OnClosed( NetLink* pLink ) = 0;
	virtual int				OnReceived( NetLink* pLink, const char* pPkt, unsigned int iLen ) = 0;
	virtual int				OnSended( NetLink* pLink, const char* pPkt, unsigned int iLen ) = 0;
	virtual int				OnError( NetLink* pLink, const char* pPkt, unsigned int iLen ) = 0;
};

//< p2p agent
struct P2PAgent
{
	virtual NetLinkMap&		GetHashMap(void ) = 0;
	virtual Network_IF&		Self( void ) = 0;
	virtual NetLink*		Connect( const Network_IF& rNIF ) = 0;
	virtual bool			Close( NetLink* pLink ) = 0;
	virtual NetLink*		Find( const Network_IF& rNIF ) = 0;
	virtual NetLink*		Relay( void ) = 0;
	virtual void			Clear( void ) = 0;
	virtual void			Process( unsigned long iWait ) = 0;
};

//< p2p agent factory
struct P2PAgentFactory
{
	static P2PAgent*		Create( const Network_IF& NIF, P2PAgentHandler* pHandler,
								unsigned int iMaxBufferSize = 4096, unsigned long iKeepConnection = 1000 );
};

//< utility
namespace P2PUtil
{
	void			SetNetworkAddress( Network_IF* pNetIF, const std::string& sIP, unsigned short iPort );
	std::string		Addr2Str( Network_IF* pNetIF );
};