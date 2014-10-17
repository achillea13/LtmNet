#ifndef __LTM_NET_AGENT__
#define __LTM_NET_AGENT__

#include <vector>
#include "common\Define.h"
#include "Interface.h"

namespace ltm_net{

class AgentServer
{

public:
	AgentServer( );
	virtual ~AgentServer();

	// 初始化服务器
	// iMaxClient		:最大连接数
	// iThreadNum		:工作线程数
	// usPort			:端口
	// fnRecvCB			:收到消息时的回调函数
	bool Init( int iMaxClient, unsigned short usPort, NetRecvCallBack fnRecvCB, NetAcceptCallBack fnAcptCB, int iThreadNum = 1 );

	// 发送一个网络数据包
	// vecID			:要发送到的客户端集合
	// pBuf				:发送的数据
	// iLen				:发送数据长度
	bool Send( std::vector<EXEC_ID> vecId, char* pBuf, int iLen );

protected:

	ServerInterface* _pNetServer;

};

class AgentClient
{
public:
	AgentClient();
	virtual ~AgentClient();

	// 初始化客户端，连接服务器
	// strIP			:服务器IP
	// usPort			:服务器端口
	// fnRecvCB			:接收到消息时的回调函数
	bool Init( const char* strIP, unsigned short usPort, NetRecvCallBack fnRecvCB );

	// 发送一个数据包到服务器
	bool Send( char* pBuf, int iLen );

protected:

	ClientInterface* _pNetClient;
};


}


#endif