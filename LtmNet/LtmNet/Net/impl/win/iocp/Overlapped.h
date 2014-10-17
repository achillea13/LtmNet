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

	// ����WSARecv()/WSASend()��buffer
	WSABUF		m_wsaBuffer;

	// send / recv ʱ��;��һ��
	bool		m_bSending;

};

};

#endif