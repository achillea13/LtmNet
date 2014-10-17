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

	// 启动客户端，连接服务器
	// strIP			:ip
	// usPort			:端口
	virtual bool Startup( const char* strIP, unsigned short usPort, NetRecvCallBack fnRecvCB );

	// 发送一个网络数据包
	// pBuf				:数据
	// iLen				:长度
	virtual bool SendData( char* pBuf, int iLen );

	// 获取socket
	SOCKET GetServerSocket(){ return _SocketServer; }

protected:

	// 线程入口调用函数
	static DWORD WINAPI _ThreadProcCB( void *pData );

	// 接收完数据的处理
	bool	OnRecvCompleted( char* pBuf, int iLen );


protected:
	SOCKET						_SocketServer;			// 服务器socket
	NetRecvCallBack*			_pfnRecvCB;				// 接收到网络消息时的回调函数
};

};




#endif