#include "stdafx.h"

//<
#include "NetLinkManager.h"
#include "Pc.h"
#include "Util.h"

//< 위로 올리지 말것
#include <Mmsystem.h>

Pc::Pc()
{
	this->m_NetLinkManager	= NULL;
	this->m_Link			= NULL;
	this->m_iSyncTm			= 0;
	this->m_iSyncCnt		= 0;
	memset( m_Latency, 0x00, sizeof(m_Latency) );
	iAvgLatency				= MIN_PACKET_RESEND_LATENCY;
}

Pc::Pc(NetLinkManager* pNetLinkMgr, UDPLink* pLink)
{
	this->m_NetLinkManager	= pNetLinkMgr;
	this->m_Link			= pLink;
	this->m_iSyncTm			= 0;
	this->m_iSyncCnt		= 0;
	memset( m_Latency, 0x00, sizeof(m_Latency) );
	iAvgLatency				= MIN_PACKET_RESEND_LATENCY;
}

Pc::~Pc()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////

Network_IF& Pc::NetIF( void )
{
	return *(Network_IF*)this;
}

void Pc::NetIF( const Network_IF& r )
{
	this->iCellID	= r.iCellID;
	this->iID		= r.iID;
	this->iAddr		= r.iAddr;
	this->iPort		= r.iPort;
}

Network_ST& Pc::NetST( void )
{
	return *(Network_ST*)this;
}

void Pc::NetST( const Network_ST& r )
{
	this->iSt					= r.iSt;
	this->iUnreliablePktSeq		= r.iUnreliablePktSeq;
	this->iReliablePktSeq		= r.iReliablePktSeq;
	this->iWishReliablePktSeq	= r.iWishReliablePktSeq;
	this->iControlPktSeq		= r.iControlPktSeq;
	this->iWishControlPktSeq	= r.iWishControlPktSeq;
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
			p->iTransTick = iTick + iAvgLatency;
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
	if ( m_iSyncCnt >= MAX_SYNC_CNT && iSt == eSYNC_ST )
	{
		iSt			= eUNLINK_ST;
		m_iSyncTm	= 0;
		m_iSyncCnt	= 0;
		return;
	}

	if ( m_iSyncTm >= timeGetTime() )
		return;

	P2PNET_PACKET_BASE* p = new P2P_SYNC(0, m_NetLinkManager->Self()->NetIF().iID);
	p->Addr.sin_family				= AF_INET;
	p->Addr.sin_port				= iPort;
	p->Addr.sin_addr.S_un.S_addr	= iAddr;

	verbose( "send sync : %s\n", Util::Addr2Str(&this->NetIF()).c_str() );
	// unreliable pkt seq 를 올리지 않기 위해서 직접 넣는다.
	m_Reserved.push_back( p );
	m_iSyncCnt++;

	m_iSyncTm = timeGetTime() + iAvgLatency;
}

void Pc::KeepConnection( void )
{
	P2PNET_PACKET_BASE* p = new P2P_KEEP_CONNECTION(0, m_NetLinkManager->Self()->NetIF().iID);
	p->Addr.sin_family				= AF_INET;
	p->Addr.sin_port				= iPort;
	p->Addr.sin_addr.S_un.S_addr	= iAddr;

	//verbose( "send keep : %s\n", Util::Addr2Str(&this->NetIF()).c_str() );
	// unreliable pkt seq 를 올리지 않기 위해서 직접 넣는다.
	m_Reserved.push_back( p );
	// next send tm
	NetST().iKeepConnection = timeGetTime() + m_NetLinkManager->m_iKeepConnection;
}


bool Pc::Push( P2PNET_PACKET_BASE* pPkt, bool bReliable )
{
#if !defined(TEST_REALY)		// 릴레이 테스트 할때
	if ( iSt == eLINK_ST )
	{
		Util::SetNetworkAddress( pPkt, NetIF() );
	}
	else
#endif
	if ( m_NetLinkManager->Relay() && m_NetLinkManager->Relay()->NetST().iSt == eLINK_ST )
	{
		verbose( "send pkt to relay %u\n", iID );
		pPkt->iToID = iID;
		Util::SetNetworkAddress( pPkt, m_NetLinkManager->Relay()->NetIF() );
	}
	else
	{
		delete[] pPkt;
		return false;
	}

	pPkt->iFromID = m_NetLinkManager->Self()->NetIF().iID;
	if ( bReliable )
	{
		pPkt->iPackID = eP2P_RELIABLE_SEND_TO;
		pPkt->iPktSeq = iReliablePktSeq++;
		m_Reliable.push_back( pPkt );
	}
	else
	{
		pPkt->iPackID = eP2P_SEND_TO;
		pPkt->iPktSeq = iUnreliablePktSeq++;
		m_Reserved.push_back( pPkt );
	}

	return true;
}

int Pc::OnReceived( P2PNET_PACKET_BASE* pPkt )
{
	NetST().iReceivedPktTm = timeGetTime();

	switch (pPkt->iPackID)
	{
	case eP2P_SYNC					:
		{
			//verbose( "recv sync %u, self %u\n", pPkt->iFromID, m_NetLinkManager->Self()->NetIF().iID  );
			P2P_SYNC_ACK p(pPkt->iCellID, m_NetLinkManager->Self()->NetIF().iID);
			p.iTransTick = pPkt->iTransTick;
#if !defined(TEST_SYNC_TIMEOUT)
			m_Link->Send((const char*)&p.iLen, p.iLen, &pPkt->Addr);
#endif
		}
		return 1;
	case eP2P_SYNC_ACK				:
		{
			Latency( NetST().iReceivedPktTm - pPkt->iTransTick );
			//verbose( "recv sync ack %u, self %u\n", pPkt->iFromID, m_NetLinkManager->Self()->NetIF().iID );
			// OnConnected 가 반복 호출방지
			if ( NetST().iSt == eSYNC_ST )
			{
				NetST().iKeepConnection = timeGetTime() + m_NetLinkManager->m_iKeepConnection;
				NetST().iSt = eLINK_ST;
				m_NetLinkManager->OnConnected( this );
			}
		}
		return 1;
	case eP2P_KEEPING_CONNECTION	:
		{
//			verbose( "recv keep connection %u\n", iID );
		}
		return 1;
	}

	return 0;
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

	iAvgLatency = iSum / (MAX_LATENCY - 2);
	if ( iAvgLatency < MIN_PACKET_RESEND_LATENCY ) iAvgLatency = MIN_PACKET_RESEND_LATENCY;
	verbose( "latency = %d\n", iAvgLatency );
}

unsigned long Pc::Latency( void )
{
	return iAvgLatency;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int Pc::SortLatency(const void *arg1, const void *arg2)
{
	if ( *(unsigned long *)arg1 < *(unsigned long *)arg2 )
		return -1;
	if ( *(unsigned long *)arg1 > *(unsigned long *)arg2 )
		return 1;
	return 0;
}