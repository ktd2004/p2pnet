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

#include <windows.h>
#include <string>
#include <FastList.h>

#define buffer_scalar_operator(y) \
	inline Buffer& operator>> ( y& Val ) \
	{ \
		if ( sizeof(y) > (int)length() ) { return *this; } \
		Val = *(reinterpret_cast<y*>(this->rd_ptr())); \
		this->rd_ptr(sizeof(Val)); \
		return *this; \
	}; \
	inline Buffer& operator<< ( y Val ) \
	{ \
		*(reinterpret_cast<y*>(this->wr_ptr())) = Val; \
		this->wr_ptr(sizeof(Val)); \
		return *this; \
	}

class Buffer : listnode(Buffer)
{
	size_t			m_ibytes;	// allocated size
	char*			m_begin;	// memory begin point
	char*			m_front;	// data read point
	char*			m_back;	// data write point
	char*			m_end;		// memory end point
	char*			m_head;	// reserved
public:
	Buffer();
	Buffer( size_t ibytes );
	Buffer( const Buffer &r );
	Buffer( const char *pBuf, size_t ibytes );
	virtual ~Buffer();

	void			resize( size_t ibytes );
	char*			reserve( size_t ibytes );
	inline void		reset( void );
	inline char*	base( void );
	inline char*	rd_ptr( void );
	inline char*	rd_ptr( size_t ipos );
	inline char*	rd_ptr( char* pos );
	inline char*	wr_ptr( void );
	inline char*	wr_ptr( size_t ipos );
	char*			wr_ptr( char* pos );
	inline size_t	length( void );
	inline size_t	size( void );
	inline size_t	capacity( void );
	inline size_t	space( void );
	unsigned int	copy( const char* pBuf, size_t ibytes );
	size_t			format( const char *pFmt, ... );
	void			crunch( void );

	inline Buffer& operator<< (const std::string& r)
	{
		*this << (unsigned int)r.size();
		this->copy(r.c_str(), r.size());
		return *this;
	}

	inline Buffer& operator>> (std::string& r)
	{
		unsigned int ibytes = 0; *this >> ibytes;
		if ( ibytes > (unsigned int)length() )
		{ return *this;	}
		r.assign( rd_ptr(), ibytes );
		this->rd_ptr( ibytes );
		return *this;
	}

	inline Buffer& operator<< (const char* pszBuf)
	{
		unsigned int ibytes = (unsigned int)::strlen(pszBuf);
		*this << ibytes;
		copy( pszBuf, ibytes );
		return *this;
	}

	inline Buffer& operator>> (char* pszBuf)
	{
		unsigned int ibytes = 0; *this >> ibytes;
		if ( ibytes > (unsigned int)length() )
		{ return *this; }
		memcpy( pszBuf, this->rd_ptr(), ibytes );
		this->rd_ptr( ibytes );
		return *this;
	}

	char&			operator[] ( size_t i );
	buffer_scalar_operator( bool );
	buffer_scalar_operator( char );
	buffer_scalar_operator( unsigned char );
	buffer_scalar_operator( short );
	buffer_scalar_operator( unsigned short );
	buffer_scalar_operator( int );
	buffer_scalar_operator( unsigned int );
	buffer_scalar_operator( long );
	buffer_scalar_operator( unsigned long );
	buffer_scalar_operator( __int64 );
	buffer_scalar_operator( unsigned __int64 );
	buffer_scalar_operator( float );
	buffer_scalar_operator( double );
};

//<
inline char* Buffer::base( void )
{
	return m_begin;
}

inline void	 Buffer::reset( void )
{
	m_front = m_back = m_begin;
}

inline char*	Buffer::rd_ptr( void )
{
	return m_front;
}

inline char*	Buffer::rd_ptr( size_t ipos )
{
	return rd_ptr( m_front + ipos );
}

inline char* Buffer::rd_ptr( char* pos )
{
	if ( pos > m_end )
	{
		m_front = m_end;
		m_back = m_end;
	}
	else if ( pos > m_back )
	{
		m_front = pos;
		m_back = pos;
	}
	else if ( pos < m_begin )
	{
		m_front = m_begin;
	}
	else
	{
		m_front = pos;
	}

	return m_front;
}

inline char* Buffer::wr_ptr( void )
{
	return m_back;
}

inline char* Buffer::wr_ptr( size_t ipos )
{
	return wr_ptr( m_back + ipos );
}

inline char* Buffer::wr_ptr( char* pos )
{
	if ( pos > m_end )
	{
		m_back = m_end;
	}
	else if ( pos < m_begin )
	{
		m_front = m_begin;
		m_back = m_begin;
	}
	else if ( pos < m_front )
	{
		m_front = pos;
		m_back = m_front;
	}
	else
	{
		m_back = pos;
	}

	return m_back;
}

inline size_t	Buffer::length( void )
{
	return m_back - m_front;
}

inline size_t	Buffer::size( void )
{
	return m_back - m_front;
}

inline size_t	Buffer::capacity( void )
{
	return m_ibytes;
}

inline size_t	Buffer::space( void )
{
	return m_end - m_back;
}

typedef util::list<Buffer>		BUFFER_LIST;