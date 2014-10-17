#ifndef __LTM_NET_WIN_CLIENT__
#define __LTM_NET_WIN_CLIENT__


#include "common/Define.h"
#include "Interface.h"
#include <winsock2.h>

namespace ltm_net{

class Client : public ClientInterface
{
public:
	Client();
	virtual ~Client();

	// �����ͻ��ˣ����ӷ�����
	// strIP			:ip
	// usPort			:�˿�
	virtual bool Startup( const char* strIP, unsigned short usPort, NetRecvCallBack fnRecvCB );

	// ����һ���������ݰ�
	// pBuf				:����
	// iLen				:����
	virtual bool SendData( char* pBuf, int iLen );

	// ��ȡsocket
	SOCKET GetServerSocket(){ return _SocketServer; }

protected:

	// �߳���ڵ��ú���
	static DWORD WINAPI _ThreadProcCB( void *pData );

	// ���������ݵĴ���
	bool	OnRecvCompleted( char* pBuf, int iLen );


protected:
	SOCKET						_SocketServer;			// ������socket
	NetRecvCallBack*			_pfnRecvCB;				// ���յ�������Ϣʱ�Ļص�����
};

};




#endif