#include "stdafx.h"

//<
#include "NetLinkManager.h"
#include "Pc.h"
#include "Util.h"

//< 위로 올리지 말것
#include <Mmsystem.h>

Pc::Pc()
{
	Init( NULL, NULL );
}

Pc::Pc(NetLinkManager* pNetLinkMgr, UDPLink* pLink)
{
	Init( pNetLinkMgr, pLink );
}

Pc::~Pc()
{
	Clear();
}

void Pc::Init( NetLinkManager* pNetLinkMgr, UDPLink* pLink )
{
	this->m_NetLinkManager	= pNetLinkMgr;
	this->m_Link			= pLink;
	memset( m_Latency, 0x00, sizeof(m_Latency) );
	this->m_iSyncTm			= 0;
	this->m_iSyncCnt		= 0;
	this->m_iSelfAddr		= m_NetLinkManager ? m_NetLinkManager->m_Self.iAddr : 0;
	this->m_iSelfPort		= m_NetLinkManager ? m_NetLinkManager->m_Self.iPort : 0;

	//<
	m_iSt					= eUNLINK_ST;
	m_iUnreliablePktSeq		= 0;
	m_iReliablePktSeq		= 0;
	m_iWishReliablePktSeq	= 0;
	m_iControlPktSeq		= 0;
	m_iWishControlPktSeq	= 0;
	m_iKeepConnection		= 0;
	m_iReceivedPktTm		= 0;
	m_iAvgLatency			= MIN_PACKET_RESEND_LATENCY;
}

//////////////////////////////////////////////////////////////////////////////////////////

Network_IF& Pc::NetIF( void )
{
	return *(Network_IF*)this;
}

void Pc::NetIF( const Network_IF& r )
{
	this->iCellID	= r.iCellID;
	this->iNID		= r.iNID;
	this->iAddr		= r.iAddr;
	this->iPort		= r.iPort;
}

eLinkST Pc::NetST( void )
{
	return m_iSt;
}

void Pc::NetST( const eLinkST r )
{
	this->m_iSt	= r;
}

bool Pc::Push( const char* pPkt, unsigned int iLen, bool bReliable )
{
	P2PNET_PACKET_BASE* p = (P2PNET_PACKET_BASE*)new char[m_Link->MaxBufferSize()];
	// P2P 헤더 설정
	p->iLen = iLen + sizeof(P2PNET_PACKET_BASE) - sizeof(SOCKADDR_IN);
	p->iTransTick = 0;
	memcpy( PKT_DATA_POS(p), pPkt, iLen );
	// 전송 요청
	return Push( p, bReliable );
}

void Pc::Process( void )
{
	P2PNET_PACKET_LIST::iterator it;
	// unreliable packet
	for ( it = m_Reserved.begin(); it != m_Reserved.end(); )
	{
		P2PNET_PACKET_BASE* p = *it;
		unsigned long iTick = timeGetTime();
		if ( p->iTransTick <= iTick )
		{
			p->iTransTick = iTick;
			if ( m_Link->Send((const char*)&p->iLen, p->iLen, &p->Addr) == -1 )
			{
				m_NetLinkManager->OnError( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
				break;
			}
			it = m_Reserved.erase(it);

			// 홀펀칭 관련 메세지 아닐 경우만
			if ( p->iPackID > eP2P_RELIABLE_ACK )
				m_NetLinkManager->OnSended( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
			delete[] p;
		}
		else
		{
			it++;
		}
	}
	// reliable packet
	for ( it = m_Reliable.begin(); it != m_Reliable.end(); it++ )
	{
		P2PNET_PACKET_BASE* p = *it;
		unsigned long iTick = timeGetTime();
		if ( p->iTransTick <= iTick )
		{
			p->iTransTick = iTick;
			if ( m_Link->Send((const char*)&p->iLen, p->iLen, &p->Addr) == -1 )
			{
				m_NetLinkManager->OnError( this, PKT_DATA_POS(p), PKT_DATA_LEN(p) );
				break;
			}
			p->iTransTick = iTick + m_iAvgLatency;
		}
	}
}

void Pc::Clear()
{
	while ( !m_Reserved.empty() )
	{
		P2PNET_PACKET_BASE* p = m_Reserved.front(); m_Reserved.pop_front();
		delete p;
	}
	while ( !m_Reliable.empty() )
	{
		P2PNET_PACKET_BASE* p = m_Reliable.front(); m_Reliable.pop_front();
		delete p;
	}
	while ( !m_WaitReliable.empty() )
	{
		P2PNET_PACKET_BASE* p = m_WaitReliable.front(); m_WaitReliable.pop_front();
		delete p;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////

void Pc::Sync( void )
{
	unsigned long iCurrTick = timeGetTime();
	if ( m_iSyncCnt >= MAX_SYNC_CNT && m_iSt == eSYNC_ST )
	{
		m_iSt		= eUNLINK_ST;
		m_iSyncTm	= 0;
		m_iSyncCnt	= 0;
		return;
	}

	if ( m_iSyncTm >= iCurrTick )
		return;

	P2PNET_PACKET_BASE* p = new P2P_SYNC(0, m_NetLinkManager->m_Self.iNID);
	p->Addr.sin_family				= AF_INET;
	p->Addr.sin_port				= iPort;
	p->Addr.sin_addr.S_un.S_addr	= iAddr;

	verbose( "send sync to %u %s\n", iNID, Util::Addr2Str(&this->NetIF()).c_str() );
	// unreliable pkt seq 를 올리지 않기 위해서 직접 넣는다.
	m_Reserved.push_back( p );
	m_iSyncCnt++;
	// next send tm
	m_iSyncTm = iCurrTick + m_iAvgLatency;
}

bool Pc::KeepConnection( void )
{
	unsigned long iCurrTick = timeGetTime();
	if ( m_iReceivedPktTm + m_NetLinkManager->m_iKeepConnection + m_iAvgLatency <= iCurrTick )
	{
		verbose( "recv pkt tm %lu, keep %lu, latency %lu, curr tm %lu\n", m_iReceivedPktTm, m_NetLinkManager->m_iKeepConnection, m_iAvgLatency, iCurrTick );
		m_NetLinkManager->OnClosed( this );
		return true;
	}

	if ( m_iKeepConnection <= iCurrTick )
	{
		return false;
	}

	P2PNET_PACKET_BASE* p = new P2P_KEEP_CONNECTION(0, m_NetLinkManager->m_Self.iNID);
	p->Addr.sin_family				= AF_INET;
	p->Addr.sin_port				= iPort;
	p->Addr.sin_addr.S_un.S_addr	= iAddr;

	//verbose( "send keep to %u %s\n", iNID, Util::Addr2Str(&this->NetIF()).c_str() );
	// unreliable pkt seq 를 올리지 않기 위해서 직접 넣는다.
	m_Reserved.push_back( p );
	// next send tm
	m_iKeepConnection = iCurrTick + m_NetLinkManager->m_iKeepConnection;

	return false;
}


bool Pc::Push( P2PNET_PACKET_BASE* pPkt, bool bReliable )
{
#if !defined(TEST_REALY)		// 릴레이 테스트 할때
	if ( m_iSt == eLINK_ST )
	{
		Util::SetNetworkAddress( pPkt, NetIF() );
	}
	else
#endif
	if ( m_NetLinkManager->Relay() && m_NetLinkManager->Relay()->NetST() == eLINK_ST )
	{
		verbose( "send pkt to relay %u\n", m_NetLinkManager->Relay()->iNID );
		pPkt->iToID = iNID;
		Util::SetNetworkAddress( pPkt, m_NetLinkManager->Relay()->NetIF() );
	}
	else
	{
		delete[] pPkt;
		return false;
	}

	pPkt->iFromID = m_NetLinkManager->m_Self.iNID;
	if ( bReliable )
	{
		pPkt->iPackID = eP2P_RELIABLE_SEND_TO;
		pPkt->iPktSeq = m_iReliablePktSeq++;
		verbose( "send from %u to %u\n", m_NetLinkManager->m_Self.iNID, iNID );
//		delete[] pPkt;
		m_Reliable.push_back( pPkt );
	}
	else
	{
		pPkt->iPackID = eP2P_SEND_TO;
		pPkt->iPktSeq = m_iUnreliablePktSeq++;
		m_Reserved.push_back( pPkt );
	}

	return true;
}

void Pc::Latency( unsigned long iLatency )
{
	memmove( m_Latency, &m_Latency[1], sizeof(m_Latency)-sizeof(unsigned long) );
	m_Latency[MAX_LATENCY-1] = iLatency;

	unsigned long Latency[MAX_LATENCY];
	unsigned long iSum = 0;

	memcpy( Latency, m_Latency, sizeof(Latency) );
	qsort( Latency, MAX_LATENCY, sizeof(unsigned long), SortLatency );

	// 최하,최상 제외하고
	for (int i = 1; i < MAX_LATENCY - 1; i++)
	{
		iSum += Latency[i];
	}

	m_iAvgLatency = iSum / (MAX_LATENCY - 2);
	if ( m_iAvgLatency < MIN_PACKET_RESEND_LATENCY ) m_iAvgLatency = MIN_PACKET_RESEND_LATENCY;
//	verbose( "latency = %lu\n", m_iAvgLatency );
}

unsigned long Pc::Latency( void )
{
	return m_iAvgLatency;
}

int Pc::SortLatency(const void *arg1, const void *arg2)
{
	if ( *(unsigned long *)arg1 < *(unsigned long *)arg2 )
		return -1;
	if ( *(unsigned long *)arg1 > *(unsigned long *)arg2 )
		return 1;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

int Pc::OnSync( P2PNET_PACKET_BASE* pPkt )
{
	m_iReceivedPktTm = timeGetTime();

	verbose( "recv sync from %u %s\n", pPkt->iFromID, Util::Addr2Str(&NetIF()).c_str() );
	P2P_SYNC_ACK pk(pPkt->iCellID, m_NetLinkManager->m_Self.iNID, pPkt->iTransTick, pPkt->Addr);
	iAddr = pPkt->Addr.sin_addr.S_un.S_addr;
	iPort = pPkt->Addr.sin_port;

#if !defined(TEST_SYNC_TIMEOUT)
	m_Link->Send((const char*)&pk.iLen, pk.iLen, &pPkt->Addr);
#endif

	delete[] pPkt;

	return 1;
}

int Pc::OnSyncAck( P2PNET_PACKET_BASE* pPkt )
{
	m_iReceivedPktTm = timeGetTime();

	Latency( m_iReceivedPktTm - pPkt->iTransTick );
	verbose( "recv sync ack from %u %s\n", pPkt->iFromID, Util::Addr2Str(&NetIF()).c_str());
	// OnConnected 가 반복 호출방지
	if ( m_iSt == eSYNC_ST )
	{
		m_iSt				= eLINK_ST;
		m_iSelfAddr			= ((P2P_SYNC_ACK*)pPkt)->iSelfAddr;
		m_iSelfPort			= ((P2P_SYNC_ACK*)pPkt)->iSelfPort;
		m_iKeepConnection	= m_iReceivedPktTm + m_NetLinkManager->m_iKeepConnection;
		m_NetLinkManager->OnConnected( this );
	}

	delete[] pPkt;

	return 1;
}

int Pc::OnKeepConnection( P2PNET_PACKET_BASE* pPkt )
{
	m_iReceivedPktTm = timeGetTime();
//	verbose( "recv keep connection self %u, from %u\n", iNID, pPkt->iFromID );
	iAddr	= pPkt->Addr.sin_addr.S_un.S_addr;
	iPort	= pPkt->Addr.sin_port;
	return 1;
}