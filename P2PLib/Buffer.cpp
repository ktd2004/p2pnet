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

#include "Buffer.h"

Buffer::Buffer()
: m_ibytes(1024)
, m_begin(NULL)
, m_front(NULL)
, m_back(NULL)
, m_end(NULL)
{
	resize( m_ibytes );
	m_head = rd_ptr();
}

Buffer::Buffer( size_t ibytes )
: m_ibytes(ibytes)
, m_begin(NULL)
, m_front(NULL)
, m_back(NULL)
, m_end(NULL)
{
	resize( m_ibytes );
	m_head = rd_ptr();
}

Buffer::Buffer( const Buffer &ref )
{
	Buffer &r = const_cast<Buffer &>(ref);
	resize( r.size() );
	copy( r.rd_ptr(), r.length() );
	m_head = rd_ptr();
}

Buffer::Buffer( const char *pBuf, size_t ibytes )
{
	resize( ibytes );
	copy( pBuf, ibytes );
	m_head = rd_ptr();
}

Buffer::~Buffer()
{
	if ( m_begin ) delete[] m_begin;
}

void Buffer::resize( size_t ibytes )
{
	char *pBuf = NULL;
	// 가장 큰값을 추려내서..
	size_t ilen = length();
	size_t icap = capacity();
	ibytes = std::max<size_t>(ibytes, std::max<size_t>(ilen, icap));

	// 버퍼가 할당되지 않았으면 메모리 할당
	if ( !m_begin )
	{
		pBuf = new char[ibytes]; pBuf[0] = 0;
		m_begin = pBuf;
		m_front = m_begin;
		m_back = m_begin;
		m_end = m_begin + ibytes;
		m_ibytes = ibytes;
	}
	// Resize 요청이 현재 할당된 저장용량 보다 크면 재할당후 데이타 복사
	else if ( icap < ibytes )
	{
		pBuf = new char[ibytes]; 
		memcpy( pBuf, m_front, ilen );
		delete[] m_begin;

		m_begin = pBuf;
		m_front = m_begin;
		m_back = m_begin + ilen;
		m_end = m_begin + ibytes;
		m_ibytes = ibytes;
	}
}

void Buffer::crunch()
{
	if ( m_front != m_begin )
	{
		size_t iDataLen = length();
		memmove( m_begin, m_front, iDataLen );
		m_front = m_begin;
		m_back = m_front + iDataLen;
		m_end = m_begin + capacity();
	}
}

char* Buffer::reserve( size_t ibytes )
{
	m_head = wr_ptr();
	wr_ptr( ibytes );
	return m_head;
}

size_t Buffer::copy( const char* pBuf, size_t ibytes )
{
	if ( ibytes > space() )
		resize( capacity() + ibytes );

	memcpy( m_back, pBuf, ibytes );
	m_back += ibytes;
	return ibytes;
}

char& Buffer::operator[]( size_t i )
{
	return rd_ptr()[i];
}

size_t Buffer::format( const char *pFmt, ... )
{
	va_list args;
	va_start( args, pFmt );
	size_t ibytes = vsnprintf_s( wr_ptr(), space(), _TRUNCATE, pFmt, args);
	wr_ptr( ibytes );
	return ibytes;
}