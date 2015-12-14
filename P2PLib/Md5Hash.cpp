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

#include <hash_map>

#include "Md5Hash.h"

///////////////////////////////////////////////////////////////////////////////////

namespace
{
	typedef void (WINAPI *fnMD5Init)	( MD5_CTX *);
	typedef void (WINAPI *fnMD5Update)	( MD5_CTX *, const unsigned char*, unsigned int32 );
	typedef void (WINAPI *fnMD5Final)	( MD5_CTX * );

	class Md5Api
	{
	protected:
		HINSTANCE		m_Inst;
		fnMD5Init		m_pFnInit;
		fnMD5Update		m_pFnUpdate;
		fnMD5Final		m_pFnFinal;

	protected:
		Md5Api() : m_Inst(NULL), m_pFnFinal(NULL), m_pFnUpdate(NULL), m_pFnInit(NULL)
		{
			m_Inst = ::LoadLibrary("Cryptdll.dll");
			if ( !m_Inst ) return;

			m_pFnInit	= reinterpret_cast<fnMD5Init>(::GetProcAddress(m_Inst, "MD5Init"));
			m_pFnUpdate = reinterpret_cast<fnMD5Update>(::GetProcAddress(m_Inst, "MD5Update"));
			m_pFnFinal	= reinterpret_cast<fnMD5Final>(::GetProcAddress(m_Inst, "MD5Final"));
		}
		Md5Api(const Md5Api &refInst)
		{
		}

		virtual ~Md5Api()
		{
			if ( m_Inst ) ::FreeLibrary(m_Inst);
		}

	public:

		static Md5Api* instance()
		{
			static Md5Api inst;
			return &inst;
		}

		static void Init(MD5_CTX* pCtx)
		{
			Md5Api::instance()->m_pFnInit(pCtx);
		}

		static void Update(MD5_CTX* pCtx, const unsigned char* pBuf, unsigned int iLen)
		{
			Md5Api::instance()->m_pFnUpdate(pCtx, pBuf, iLen);
		}

		static void Final(MD5_CTX *pCtx)
		{
			Md5Api::instance()->m_pFnFinal(pCtx);
		}
	};

	class EncryptTable
	{
	public:
		unsigned int m_Data[1280];
		stdext::hash_map<const char *, unsigned int> m_HashMap;
		stdext::hash_map<unsigned int, std::string> m_StringMap;

	public:
		EncryptTable()
		{
			memset(m_Data, 0x00, sizeof(m_Data));
			unsigned int seed = 0x00100001, index1 = 0, index2 = 0, i;

			for(index1 = 0; index1 < 0x100; index1++)
			{ 
				for(index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
				{ 
					unsigned int temp1, temp2;

					seed = (seed * 125 + 3) % 0x2AAAAB;
					temp1 = (seed & 0xFFFF) << 0x10;

					seed = (seed * 125 + 3) % 0x2AAAAB;
					temp2 = (seed & 0xFFFF);

					m_Data[index2] = (temp1 | temp2); 
				}
			}
		}
	};

	const unsigned int HASH_TYPE	= 0x2;
	const unsigned int HASH_SEED1	= 0x7FED7FEDu;
	const unsigned int HASH_SEED2	= 0xEEEFEEEFu;
	static EncryptTable s_Table;

	// 문자열 해쉬 값을 캐쉬하여 수행속도를 높인 문자열해쉬함수
	// 문자열의 길이에 따라 수행속도가 결정된다.
	// 문자열의 길이가 49byte 이상일 경우에 성능이 좋다.
	unsigned int StrHash(const char *pStr, int len)
	{
		if ( len == 0 ) len = (int)strlen( pStr );
		unsigned int seed1 = HASH_SEED1, seed2 = HASH_SEED2;
		const char * key = pStr;
		for(register int i=0; i<len; i++ )
		{
			unsigned char ch = (unsigned char)*(key+i);
			seed1 = s_Table.m_Data[(HASH_TYPE<<8)+ch] ^ (seed1+seed2);
			seed2 = ch+seed1+seed2+(seed2<<5)+3;
		}

		return seed1;
	}

}

///////////////////////////////////////////////////////////////////////////////////

Md5Result::Md5Result()
{
	memset( m_Data, 0x00, sizeof(m_Data) );
}

std::string Md5Result::ToString() const
{
	std::string rst;
	char szBuf[9];
	for ( size_t i=0; i < 16; ++i )
	{
		sprintf_s(szBuf, sizeof(szBuf), "%02X", (unsigned char)m_Data[i]);
		rst += szBuf;
	}
	return rst;
}

bool Md5Result::FromString( const std::string& sData )
{
	if( sData.size() != 16*2 )
	{
		return false;
	}

	unsigned int val;
	for ( size_t i = 0; i < 16; ++i )
	{
		std::string sSub = sData.substr(i*2, 2);
		if( sscanf_s(sSub.c_str(), "%02X", &val) != 1 )
		{
			m_Data[i] = 0;
		}
		else
		{
			m_Data[i] = static_cast<unsigned char>(val);
		}
	}
	return true;
}

bool Md5Result::operator==( const Md5Result & r ) const
{
	return memcmp(m_Data, r.m_Data, 16) == 0;
}

///////////////////////////////////////////////////////////////////////////////////

Md5Hash::Md5Hash(void)
{
	Md5Api::Init( &m_Ctx );
}

Md5Hash::~Md5Hash(void)
{
}

void Md5Hash::Append( const void * buf, size_t len )
{
	Md5Api::Update(&m_Ctx, reinterpret_cast<const unsigned char*>(buf), static_cast<unsigned int>(len));
}

Md5Result Md5Hash::Finish()
{
	Md5Result rst;
	Md5Api::Final(&m_Ctx);
	memcpy( rst.m_Data, m_Ctx.digest, sizeof(rst.m_Data) );
	return rst;
}

Md5Result Md5Hash::FromFile( const std::string& sFileName )
{
	FILE * file = _fsopen(sFileName.c_str(), "rb", _SH_DENYNO);
	if( file == NULL )
	{
		return Md5Result();
	}

	Md5Hash hasher;

	enum { BUF_LEN = 1024 * 16 };
	char buf[BUF_LEN];
	for(;;)
	{
		size_t n = fread(buf, 1, BUF_LEN, file);
		if( n != BUF_LEN )
		{
			if( n < BUF_LEN && n > 0 )
			{
				hasher.Append(buf, n);
			}
			break;
		}
		else
		{
			hasher.Append(buf, n);
		}
	}

	fclose(file);

	return hasher.Finish();
}

Md5Result Md5Hash::FromString( const std::string& sData )
{
	Md5Hash hasher;
	hasher.Append(sData.data(), sData.size());

	return hasher.Finish();
}

unsigned int Md5Hash::ToNumber( const std::string& sData )
{
	return StrHash( sData.c_str(), (int)sData.size() );
}