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

// array reliable ack latency
#define MAX_LATENCY						10
// reliable packet resend
#define MIN_PACKET_RESEND_LATENCY		100
// packet receive time
#define MAX_PACKET_RECEIVE_TM			50
// max sync time
#define MAX_SYNC_CNT					3

#if defined(_DEBUG)
#define	verbose(fmt,...)		printf( fmt, __VA_ARGS__ )
#else
#define	verbose(fmt,...)
#endif

// relay test
//#define TEST_REALY
// sync timeout test
//#define TEST_SYNC_TIMEOUT
