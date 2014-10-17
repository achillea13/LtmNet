#ifndef __LTM_NET_OVERLAPPED__
#define __LTM_NET_OVERLAPPED__

#include <WinSock2.h>

ltm_net{

class MOVERLAPPED : public OVERLAPPED
{

public:

	enum IO_MODE
	{
		IO_RECV,
		IO_SEND,
	};

	MOVERLAPPED()
	{
		hEvent		= NULL;
		Clear();
		m_bSending	= false;
	}

	void Clear()
	{
		Internal		= 0;
		InternalHigh	= 0;
		Offset			= 0;
		OffsetHigh		= 0;
	}



public:

	IO_MODE		m_eIOMode;

	// 用于WSARecv()/WSASend()的buffer
	WSABUF		m_wsaBuffer;

	// send / recv 时用途不一样
	bool		m_bSending;

};

};

#endif