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
		// UnusedID��ʼ��ʧ�ܣ�������־


		return false;
	}

	// ����ID����
	for ( EXEC_ID i = 1; i <= iMaxClient; i++ )
	{
		m_mapSockObj[i] = 0;
	}

	m_pfnRecvCB = fnRecvCB;
	m_pfnAcptCB = fnAcptCB;


	// ������ɶ˿�
	WSADATA wsaData;
	DWORD ret;
	if ( ret = WSAStartup( WSA_VERSION_, &wsaData ) != 0 )
	{
		// ����ʧ�ܣ�������־
		return false;
	}

	m_hCompletionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
	if ( m_hCompletionPort == NULL )
	{
		// ������ɶ˿�ʧ�ܣ�������־
		return false;
	}
	
	// ���������߳�
	for ( int i = 0; i < iThreadNum; i++ )
	{
		HANDLE hThread = NULL;
		DWORD dwThreadID = 0;
		hThread = CreateThread( NULL,	0,	_ThreadProcCB,	(void*)this,	0,	&dwThreadID );
		if ( hThread == NULL )
		{
			// ������־�������߳�ʧ��
			return false;
		}

		m_mapThread[dwThreadID] = hThread;
	}



	m_sock	= ::WSASocket( AF_INET,	SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if ( m_sock == INVALID_SOCKET )
	{
		// ʧ�ܣ�������־

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
		// ��ʧ�ܣ�������־

		CLOSE_SOCKET( m_sock );
		::WSACleanup();

		return false;

	}

	if ( SOCKET_ERROR == listen( m_sock, 5 ) )
	{
		// ����ʧ�ܣ�������־

		CLOSE_SOCKET( m_sock );
		::WSACleanup();

		return false;
	}

	
	
	// ��ʼ��һ��Overlapped
	MOVERLAPPED ol;
	ol.Clear();
	ol.m_wsaBuffer.len		= WSABUFF_LEN_LMT;
	ol.m_wsaBuffer.buf		= ol.m_cBuffer;
	ol.m_eMode				= MOVERLAPPED::IO_RECV;

	// ��ʼ��ѭ������������
	while ( 1 )
	{
		SOCKET acceptSocket = NULL;
		acceptSocket = WSAAccept( m_sock, NULL, NULL, NULL, 0 );
		if ( acceptSocket == SOCKET_ERROR )
		{
			// ʧ�ܣ�������־

			m_pfnAcptCB( 0, AEC_UNKNOW, NULL, 0 );
			continue;
		}

		DWORD dwUnusedID = m_pUnusedID->GetUnusedID();
		if ( dwUnusedID == 0 )
		{
			// �ﵽ���ޣ�������־

			m_pfnAcptCB( 0, AEC_NUM_LMT, NULL, 0 );
			continue;
		}

		if ( CreateIoCompletionPort( (HANDLE)acceptSocket, m_hCompletionPort, dwUnusedID, 0 ) == NULL )
		{
			// ʧ�ܣ�������־

			m_pfnAcptCB( 0, AEC_COMPORT_FAIL, NULL, 0 );
			continue;
		}

		// ����SocketObject
		SocketObject* pSockObj = NULL;
		pSockObj = new SocketObject();
		if ( pSockObj == NULL )
		{
			// ʧ�ܣ�������־

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
				// Ͷ��ʧ�ܣ�������־

				// ��������Ч�����������
				Disconnect( dwUnusedID );

				m_pfnAcptCB( 0, AEC_WSARECV_PENDING, NULL, 0 );
				continue;
			}
			else
			{
				// Ͷ�ݳɹ���������־
				// ����һ�����ӣ� ֪ͨ�ⲿ
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

	HANDLE completionPort = pThis->GetCompletionPort();			// ��ɶ˿�
	DWORD dwTransferred;										// ������
	DWORD dwExecID = 0;											// ID


	while ( 1 )
	{

		MOVERLAPPED recvOl;
		recvOl.Clear();
		recvOl.m_wsaBuffer.buf	= recvOl.m_cBuffer;
		recvOl.m_wsaBuffer.len	= WSABUFF_LEN_LMT;
		recvOl.m_eMode			= MOVERLAPPED::IO_RECV;
		MOVERLAPPED* pRecvOl = &recvOl;
		DWORD recvBytes = 0;				// ���մ�С
		DWORD flags;
		if ( GetQueuedCompletionStatus( completionPort, &dwTransferred, &dwExecID, (LPOVERLAPPED*)(&pRecvOl), INFINITE ) == 0 )
		{
			// ʧ�ܣ�������־

			continue;
		}

		// ����Ƿ�������
		if ( dwTransferred == 0 )
		{
			// ������ ��

			continue;
		}

		if ( dwTransferred <= WSABUFF_LEN_LMT )
			pRecvOl->m_wsaBuffer.buf[dwTransferred] = '\0';

		// ���execid
		if ( dwExecID == 0 )
		{
			// ʧ�ܣ�������־

			continue;
		}


		// ״̬
		switch( pRecvOl->m_eMode )
		{
		case MOVERLAPPED::IO_RECV:
			{

				pThis->OnRecvCompleted(dwExecID, pRecvOl);

				// �ٷ���һ������
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

	// ���ûص������������յ����ݴ���ȥ
	m_pfnRecvCB( id, pOL->m_wsaBuffer.buf, strlen(pOL->m_wsaBuffer.buf));
				

	SocketObject* pSockObj = GetSocketObject(id);
	if ( pSockObj == NULL )
		return false;

	// Ͷ��recv��iocp����
	DWORD recvBytes = 0;
	DWORD flags = 0;
	if ( WSARecv( pSockObj->_socket, &pOL->m_wsaBuffer, 1, &recvBytes, &flags, &pOL->m_ol, NULL ) == SOCKET_ERROR )
	{
		if ( WSAGetLastError() != ERROR_IO_PENDING )
		{
			// ʧ�ܣ�������־

			return false;
		}
	}


	return true;
}


bool Server::SendData( std::vector<EXEC_ID> vecId, char* pBuf, int iLen )
{
	if ( vecId.empty() )
		return false;

	MOVERLAPPED pSendOl;						// ����overlapped
	pSendOl.Clear();
	pSendOl.m_wsaBuffer.buf	= pSendOl.m_cBuffer;
	pSendOl.m_wsaBuffer.len	= iLen;
	pSendOl.m_eMode			= MOVERLAPPED::IO_SEND;

	memcpy( pSendOl.m_wsaBuffer.buf, pBuf, iLen );

	DWORD sendBytes = 0;


	// �������ݳ�ȥ
	for ( int i = 0; i < vecId.size(); i++ )
	{
		
		SocketObject* pSockObj = GetSocketObject(vecId[i]);
		if ( pSockObj == NULL )
			return false;

		if ( WSASend( pSockObj->_socket, &pSendOl.m_wsaBuffer, 1, &sendBytes, 0, &pSendOl.m_ol, NULL ) == SOCKET_ERROR )
		{
			if ( WSAGetLastError() != ERROR_IO_PENDING )
			{
				// ʧ�ܣ�������־

				continue;
			}
			else
			{
				// Ͷ�ݳɹ�
			}
		}		
	}


	return true;
}


bool Server::Disconnect( EXEC_ID id )
{
	// problem :���ܻ����һ�����⣬
	//			������߳�����ʹ�ô����ӵ�ʱ��������ת���˴���
	//			�����˴����ӣ�����ת��ȥ�󣬿�ָ����������������
	//			�˴�����Ҫ��֤m_mapSockObj�Ķ����ռ

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