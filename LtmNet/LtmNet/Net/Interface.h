#ifndef __LTM_NET_INTERFACE__
#define __LTM_NET_INTERFACE__


#include "common\Define.h"
#include <vector>

namespace ltm_net{

class ServerInterface
{
public:
	ServerInterface(){};
	virtual ~ServerInterface(){};

	// 启动服务器
	// iMaxClient		:最大连接数
	// iThreadNum		:工作线程数
	// usPort			:端口
	// fnRecvCB			:接收到网络数据包时的回调函数
	virtual bool Startup( int iMaxClient, unsigned short usPort, NetRecvCallBack fnRecvCB, NetAcceptCallBack fnAcptCB, int iThreadNum ) = 0;

	// 发送一个网络数据包
	// vecID			:要发送到的客户端集合
	// pBuf				:发送的数据
	// iLen				:发送数据长度
	virtual bool SendData( std::vector<EXEC_ID> vecId, char* pBuf, int iLen ) = 0;

	// 断开一个连接
	// id				:客户端ID
	virtual bool Disconnect( EXEC_ID id ) = 0;
};



class ClientInterface
{
public:
	ClientInterface(){};
	virtual ~ClientInterface(){};

	// 启动客户端，连接服务器
	// strIP			:ip
	// usPort			:端口
	virtual bool Startup( const char* strIP, unsigned short usPort, NetRecvCallBack fnRecvCB ) = 0;

	// 发送一个网络数据包
	// pBuf				:数据
	// iLen				:长度
	virtual bool SendData( char* pBuf, int iLen ) = 0;

};

};

#endif