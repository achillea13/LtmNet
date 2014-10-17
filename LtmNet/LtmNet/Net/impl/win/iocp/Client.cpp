#include "Client.h"


#pragma comment( lib, "Ws2_32.lib" )

namespace ltm_net{

Client::Client()
{
	_pfnRecvCB = NULL;
}

Client::~Client()
{
	closesocket( _SocketServer );
}

bool Client::Startup( const char* strIP, unsigned short usPort, NetRecvCallBack fnRecvCB )
{
	WSAData wsaData;
	sockaddr_in addrServer;

    if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
    {
        // 加入日志，"WSAStartup失败"
        return false;
    }

    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        // 加入日志，"SOCKET版本不对"
        WSACleanup();
        return false;
    }

	_SocketServer = socket(AF_INET,SOCK_STREAM,0);
//	unsigned long iMode = 1;
//	ioctlsocket( _SocketServer, FIONBIO, &iMode );				// 设置为非阻塞模式socket
    if( _SocketServer == INVALID_SOCKET )
    {
        // 加入日志，"socket 失败"
        WSACleanup();
        return false;
    }

    memset(&addrServer,0,sizeof(sockaddr_in));
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = inet_addr(strIP);
    addrServer.sin_port = htons(usPort);

    // 连接服务器
    if(connect(_SocketServer,(const struct sockaddr *)&addrServer,sizeof(sockaddr)) != 0)
    {
        // 加入日志，"connect 失败"
        WSACleanup();
        return false;
    }

    // 连接成功
	_pfnRecvCB	= fnRecvCB;

	// 开启接收
	HANDLE hThread = NULL;
	DWORD dwThreadID = 0;
	hThread = CreateThread( NULL,	0,	_ThreadProcCB,	(void*)this,	0,	&dwThreadID );
	if ( hThread == NULL )
	{
		// 加入日志，创建线程失败
		return false;
	}

	return true;
}



bool Client::SendData( char* pBuf, int iLen )
{

	if(send(_SocketServer,pBuf,iLen,0) <= 0)
    {
		// 加入日志，发送失败
        return false;
    }

	return true;
}



DWORD WINAPI Client::_ThreadProcCB( void *pData )
{
	
	if ( pData == NULL )
		return 0;

	Client* pThis = (Client*)pData;
	if ( pThis == NULL )
		return 0;

	int iRes = 0;
	char cRecvBuff[WSABUFF_LEN_LMT] = {0};
	int iRecvBuffLen = WSABUFF_LEN_LMT;
	do
	{
		iRes = recv( pThis->GetServerSocket(), cRecvBuff, iRecvBuffLen, 0 );
		if ( iRes > 0 )
		{
			// do
			pThis->OnRecvCompleted( cRecvBuff, iRecvBuffLen );
		}
		else if ( iRes == 0 )
		{
			// 加入日志，connection closed
		}
		else
		{
			// 加入日志，recv failed
		}

	} while ( iRes > 0 );

	return 0;

}

bool Client::OnRecvCompleted( char* pBuf, int iLen )
{
	// 调用回调函数，将接收的数据传出去
	_pfnRecvCB( 0, pBuf, strlen(pBuf));
				
	return true;
}

}