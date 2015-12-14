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

//<
struct Md5Result
{
	unsigned char m_Data[16];
	Md5Result();
	std::string ToString() const;
	bool		FromString( const std::string& sData );

	bool operator == (const Md5Result & r) const;
	bool operator != (const Md5Result & r) const
	{
		return !(operator ==(r));
	}
};

//<
typedef struct
{
	ULONG			i[2];
	ULONG			buf[4];
	unsigned char	in[64];
	unsigned char	digest[16];
} MD5_CTX;

class Md5Hash
{
private:
	MD5_CTX m_Ctx;

public:
	Md5Hash();
	virtual ~Md5Hash();

	void		Append( const void * pBuf, size_t iLen );
	Md5Result	Finish();

	static Md5Result FromFile( const std::string& sFileName );
	static Md5Result FromString( const std::string& sData );
	static unsigned int ToNumber( const std::string& sData );
};