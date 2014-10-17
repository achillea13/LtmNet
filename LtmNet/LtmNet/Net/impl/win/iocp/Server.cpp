#include "Server.h"



#pragma comment( lib, "Ws2_32.lib" )


namespace ltm_net{

#define WSA_VERSION_ 0x0202

Server::Server(void)
{

	m_sock		= NULL;
	m_pUnusedID	= NULL;

	m_pfnRecvCB		= NULL;
	m_pfnAcptCB		= NULL;

}


Server::~Server(void)
{
	SAFE_DELETE( m_pUnusedID );

	std::map< EXEC_ID, SocketObject* >::iterator it = m_mapSockObj.begin();
	for ( ; it != m_mapSockObj.end(); it++ )
	{
		SAFE_DELETE( it->second );
	}
	m_mapSockObj.clear();
}



bool Server::Startup( int iMaxClient, unsigned short usPort, NetRecvCallBack fnRecvCB, NetAcceptCallBack fnAcptCB, int iThreadNum )
{
	if ( fnRecvCB == NULL || fnAcptCB == NULL )
		return false;

	m_pUnusedID = new UnusedID( iMaxClient );
	if ( m_pUnusedID == NULL )
	{
		// UnusedID初始化失败，加入日志


		return false;
	}

	// 构造ID队列
	for ( EXEC_ID i = 1; i <= iMaxClient; i++ )
	{
		m_mapSockObj[i] = 0;
	}

	m_pfnRecvCB = fnRecvCB;
	m_pfnAcptCB = fnAcptCB;


	// 创建完成端口
	WSADATA wsaData;
	DWORD ret;
	if ( ret = WSAStartup( WSA_VERSION_, &wsaData ) != 0 )
	{
		// 启动失败，加入日志
		return false;
	}

	m_hCompletionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
	if ( m_hCompletionPort == NULL )
	{
		// 创建完成端口失败，加入日志
		return false;
	}
	
	// 创建工作线程
	for ( int i = 0; i < iThreadNum; i++ )
	{
		HANDLE hThread = NULL;
		DWORD dwThreadID = 0;
		hThread = CreateThread( NULL,	0,	_ThreadProcCB,	(void*)this,	0,	&dwThreadID );
		if ( hThread == NULL )
		{
			// 加入日志，创建线程失败
			return false;
		}

		m_mapThread[dwThreadID] = hThread;
	}



	m_sock	= ::WSASocket( AF_INET,	SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if ( m_sock == INVALID_SOCKET )
	{
		// 失败，加入日志

		::WSACleanup();
		return false;
	}


	SOCKADDR_IN		local;
	memset( &local, 0 ,sizeof(SOCKADDR_IN) );
	local.sin_family		= AF_INET;
	local.sin_addr.s_addr	= htonl(INADDR_ANY);
	local.sin_port			= htons( usPort );

	if ( ::bind( m_sock, (PSOCKADDR)&local, sizeof(SOCKADDR) ) == SOCKET_ERROR )
	{
		// 绑定失败，加入日志

		CLOSE_SOCKET( m_sock );
		::WSACleanup();

		return false;

	}

	if ( SOCKET_ERROR == listen( m_sock, 5 ) )
	{
		// 监听失败，加入日志

		CLOSE_SOCKET( m_sock );
		::WSACleanup();

		return false;
	}

	
	
	// 初始化一个Overlapped
	MOVERLAPPED ol;
	ol.Clear();
	ol.m_wsaBuffer.len		= WSABUFF_LEN_LMT;
	ol.m_wsaBuffer.buf		= ol.m_cBuffer;
	ol.m_eMode				= MOVERLAPPED::IO_RECV;

	// 开始死循环，接受连接
	while ( 1 )
	{
		SOCKET acceptSocket = NULL;
		acceptSocket = WSAAccept( m_sock, NULL, NULL, NULL, 0 );
		if ( acceptSocket == SOCKET_ERROR )
		{
			// 失败，加入日志

			m_pfnAcptCB( 0, AEC_UNKNOW, NULL, 0 );
			continue;
		}

		DWORD dwUnusedID = m_pUnusedID->GetUnusedID();
		if ( dwUnusedID == 0 )
		{
			// 达到上限，加入日志

			m_pfnAcptCB( 0, AEC_NUM_LMT, NULL, 0 );
			continue;
		}

		if ( CreateIoCompletionPort( (HANDLE)acceptSocket, m_hCompletionPort, dwUnusedID, 0 ) == NULL )
		{
			// 失败，加入日志

			m_pfnAcptCB( 0, AEC_COMPORT_FAIL, NULL, 0 );
			continue;
		}

		// 创建SocketObject
		SocketObject* pSockObj = NULL;
		pSockObj = new SocketObject();
		if ( pSockObj == NULL )
		{
			// 失败，加入日志

			m_pfnAcptCB( 0, AEC_SOCKOBJ_FAIL, NULL, 0 );
			continue;
		}
		pSockObj->_socket			= acceptSocket;
		pSockObj->_id				= dwUnusedID;
		m_mapSockObj[dwUnusedID]	= pSockObj;

		DWORD recvByte = 0;
		DWORD flag = 0;


		if ( WSARecv( acceptSocket, &(ol.m_wsaBuffer), 1, &recvByte, &flag, &(ol.m_ol), NULL ) == SOCKET_ERROR )
		{
			int iEror = WSAGetLastError();
			if ( iEror != ERROR_IO_PENDING )
			{
				// 投递失败，加入日志

				// 此连接无效，清理此连接
				Disconnect( dwUnusedID );

				m_pfnAcptCB( 0, AEC_WSARECV_PENDING, NULL, 0 );
				continue;
			}
			else
			{
				// 投递成功，加入日志
				// 新增一个连接， 通知外部
				m_pfnAcptCB( dwUnusedID, AEC_OK, NULL, 0 );

			}
		}

	

	}
	


	return true;
}



DWORD WINAPI Server::_ThreadProcCB( void *pData )
{
	
	if ( pData == NULL )
		return 0;

	Server* pThis = (Server*)pData;
	if ( pThis == NULL )
		return 0;

	HANDLE completionPort = pThis->GetCompletionPort();			// 完成端口
	DWORD dwTransferred;										// 传输量
	DWORD dwExecID = 0;											// ID


	while ( 1 )
	{

		MOVERLAPPED recvOl;
		recvOl.Clear();
		recvOl.m_wsaBuffer.buf	= recvOl.m_cBuffer;
		recvOl.m_wsaBuffer.len	= WSABUFF_LEN_LMT;
		recvOl.m_eMode			= MOVERLAPPED::IO_RECV;
		MOVERLAPPED* pRecvOl = &recvOl;
		DWORD recvBytes = 0;				// 接收大小
		DWORD flags;
		if ( GetQueuedCompletionStatus( completionPort, &dwTransferred, &dwExecID, (LPOVERLAPPED*)(&pRecvOl), INFINITE ) == 0 )
		{
			// 失败，加入日志

			continue;
		}

		// 检查是否有数据
		if ( dwTransferred == 0 )
		{
			// 空数据 ？

			continue;
		}

		if ( dwTransferred <= WSABUFF_LEN_LMT )
			pRecvOl->m_wsaBuffer.buf[dwTransferred] = '\0';

		// 检查execid
		if ( dwExecID == 0 )
		{
			// 失败，加入日志

			continue;
		}


		// 状态
		switch( pRecvOl->m_eMode )
		{
		case MOVERLAPPED::IO_RECV:
			{

				pThis->OnRecvCompleted(dwExecID, pRecvOl);

				// 再发送一个测试
//				std::vector<EXEC_ID> vecID;
//				vecID.push_back(dwExecID);
//				pThis->SendData( vecID, pRecvOl->m_cBuffer, strlen(pRecvOl->m_cBuffer) );

			}
			break;

		case MOVERLAPPED::IO_SEND:
			{

			}
			break;
		}
	}


	return 0;

}











bool Server::OnRecvCompleted(EXEC_ID id, MOVERLAPPED *pOL)
{

	// 调用回调函数，将接收的数据传出去
	m_pfnRecvCB( id, pOL->m_wsaBuffer.buf, strlen(pOL->m_wsaBuffer.buf));
				

	SocketObject* pSockObj = GetSocketObject(id);
	if ( pSockObj == NULL )
		return false;

	// 投递recv到iocp队列
	DWORD recvBytes = 0;
	DWORD flags = 0;
	if ( WSARecv( pSockObj->_socket, &pOL->m_wsaBuffer, 1, &recvBytes, &flags, &pOL->m_ol, NULL ) == SOCKET_ERROR )
	{
		if ( WSAGetLastError() != ERROR_IO_PENDING )
		{
			// 失败，加入日志

			return false;
		}
	}


	return true;
}


bool Server::SendData( std::vector<EXEC_ID> vecId, char* pBuf, int iLen )
{
	if ( vecId.empty() )
		return false;

	MOVERLAPPED pSendOl;						// 发送overlapped
	pSendOl.Clear();
	pSendOl.m_wsaBuffer.buf	= pSendOl.m_cBuffer;
	pSendOl.m_wsaBuffer.len	= iLen;
	pSendOl.m_eMode			= MOVERLAPPED::IO_SEND;

	memcpy( pSendOl.m_wsaBuffer.buf, pBuf, iLen );

	DWORD sendBytes = 0;


	// 发送数据出去
	for ( int i = 0; i < vecId.size(); i++ )
	{
		
		SocketObject* pSockObj = GetSocketObject(vecId[i]);
		if ( pSockObj == NULL )
			return false;

		if ( WSASend( pSockObj->_socket, &pSendOl.m_wsaBuffer, 1, &sendBytes, 0, &pSendOl.m_ol, NULL ) == SOCKET_ERROR )
		{
			if ( WSAGetLastError() != ERROR_IO_PENDING )
			{
				// 失败，加入日志

				continue;
			}
			else
			{
				// 投递成功
			}
		}		
	}


	return true;
}


bool Server::Disconnect( EXEC_ID id )
{
	// problem :可能会存在一个问题，
	//			当别的线程正在使用此连接的时候，正好跳转到此处，
	//			销毁了此连接，则跳转回去后，空指针对象可能引发崩溃
	//			此处必须要保证m_mapSockObj的对象独占

	std::map< EXEC_ID, SocketObject* >::iterator it = m_mapSockObj.find(id);
	if ( it == m_mapSockObj.end() )
		return true;

	SAFE_DELETE( it->second );
	m_mapSockObj.erase( it );
	if ( m_pUnusedID != NULL )
	{
		m_pUnusedID->GiveBackID( id );
	}
	
	return true;

}





};