#ifndef __LTM_NET_WIN_SERVER__
#define __LTM_NET_WIN_SERVER__

/*----------------------------------------------------------------*/
/*----		使用IOCP网络通信			----*/
/*----------------------------------------------------------------*/

#include <map>
#include <queue>
#include <vector>
#include <WinSock2.h>

#include "common/Define.h"

#include "common/UnusedID.h"

#include "Interface.h"


namespace ltm_net{

#define CLOSE_SOCKET(socket)		\
			if (socket != INVALID_SOCKET)	\
			{								\
			::shutdown(socket, SD_BOTH);	\
			::closesocket( socket );		\
			socket = INVALID_SOCKET;		\
			}								





class Server : public ServerInterface
{

public:
	
	struct MOVERLAPPED
	{
	public:
		enum IO_MODE
		{
			IO_RECV,
			IO_SEND,
		};

		MOVERLAPPED()
		{
			Clear();
		}

		void Clear()
		{
			m_ol.hEvent = NULL;
			m_ol.Internal	= 0;
			m_ol.InternalHigh	= 0;
			m_ol.Offset		= 0;
			m_ol.OffsetHigh		= 0;
		}


	public:
		OVERLAPPED m_ol;

		// 用于WSARecv/WSASend的buffer
		WSABUF		m_wsaBuffer;

		char		m_cBuffer[WSABUFF_LEN_LMT];

		// 读写状态
		IO_MODE		m_eMode;

	};

	struct SocketObject
	{

	public:
		SocketObject()
		{
			_socket = 0;

			_id = 0;
		}

	public:
		EXEC_ID		_id;
		SOCKET		_socket;
	};


public:
	Server(void);
	virtual ~Server(void);

	
	// 启动服务器
	// iMaxClient		:最大连接数
	// iThreadNum		:工作线程数
	// usPort			:端口
	// fnRecvCB			:接收到网络数据包时的回调函数
	virtual bool Startup( int iMaxClient, unsigned short usPort, NetRecvCallBack fnRecvCB, NetAcceptCallBack fnAcptCB, int iThreadNum = 1 );




	// 发送一个网络数据包
	// vecID			:要发送到的客户端集合
	// pBuf				:发送的数据
	// iLen				:发送数据长度
	virtual bool SendData( std::vector<EXEC_ID> vecId, char* pBuf, int iLen );

	// 断开一个连接
	// id				:客户端ID
	virtual bool Disconnect( EXEC_ID id );


private:

	HANDLE GetCompletionPort(){ return m_hCompletionPort; }

	SocketObject* GetSocketObject(EXEC_ID id){ return m_mapSockObj[id]; }

protected:

	// 线程入口调用函数
	static DWORD WINAPI _ThreadProcCB( void *pData );

	// 接收到消息时的逻辑
	bool	OnRecvCompleted(EXEC_ID id, MOVERLAPPED *pOL);

private:

	std::map< EXEC_ID, SocketObject* >		m_mapSockObj;	// 连接的套接字 first exeid		second SocketObject

	std::map< DWORD, HANDLE >		m_mapThread;			// 线程列表 first Thread ID		second Thread Handle

	SOCKET							m_sock;					// 监听socket

	HANDLE							m_hCompletionPort;		// 完成端口

	UnusedID*						m_pUnusedID;			// ID控制器

	NetRecvCallBack*					m_pfnRecvCB;				// 接收到网络消息时的回调函数
	NetAcceptCallBack*					m_pfnAcptCB;				// 成功接收到客户端连接时的回调函数

};

};


#endif