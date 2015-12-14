#pragma once

//<
#include <NetLink.h>

//<
#include <hash_map>
typedef stdext::hash_map<unsigned int, NetLink*>	NetLinkMap;

struct P2PAgentHandler
{
	virtual int				OnConnected( NetLink* pLink ) = 0;
	virtual int				OnClosed( NetLink* pLink ) = 0;
	virtual int				OnReceived( NetLink* pLink, const char* pPkt, unsigned int iLen ) = 0;
	virtual int				OnSended( NetLink* pLink, const char* pPkt, unsigned int iLen ) = 0;
	virtual int				OnError( NetLink* pLink, const char* pPkt, unsigned int iLen ) = 0;
};

//<
struct P2PAgent
{
	virtual NetLinkMap&		GetHashMap(void ) = 0;
	virtual NetLink*		Self( const Network_IF& rNetIF ) = 0;
	virtual NetLink*		Self( void ) = 0;
	virtual NetLink*		Insert( const Network_IF& rNetIF ) = 0;
	virtual bool			Erase( const Network_IF& rNetIF ) = 0;
	virtual NetLink*		Find( const Network_IF& rNetIF ) = 0;
	virtual NetLink*		Relay( void ) = 0;
	virtual void			Clear( void ) = 0;
	virtual void			Process( void ) = 0;
};

//<
struct P2PAgentFactory
{
	static P2PAgent*		Create( P2PAgentHandler* pHandler, const std::string& sIP, unsigned short iPort,
								unsigned int iMaxBufferSize = 4096, unsigned long iKeepConnection = 1000 );
};

namespace P2PUtil
{
	void			SetNetworkAddress( Network_IF* pNetIF, const std::string& sIP, unsigned short iPort );
	std::string		Addr2Str( Network_IF* pNetIF );
};