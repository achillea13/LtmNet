#ifndef __LTM_NET_WIN_SERVER__
#define __LTM_NET_WIN_SERVER__

/*----------------------------------------------------------------*/
/*----		ʹ��IOCP����ͨ��			----*/
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

		// ����WSARecv/WSASend��buffer
		WSABUF		m_wsaBuffer;

		char		m_cBuffer[WSABUFF_LEN_LMT];

		// ��д״̬
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

	
	// ����������
	// iMaxClient		:���������
	// iThreadNum		:�����߳���
	// usPort			:�˿�
	// fnRecvCB			:���յ��������ݰ�ʱ�Ļص�����
	virtual bool Startup( int iMaxClient, unsigned short usPort, NetRecvCallBack fnRecvCB, NetAcceptCallBack fnAcptCB, int iThreadNum = 1 );




	// ����һ���������ݰ�
	// vecID			:Ҫ���͵��Ŀͻ��˼���
	// pBuf				:���͵�����
	// iLen				:�������ݳ���
	virtual bool SendData( std::vector<EXEC_ID> vecId, char* pBuf, int iLen );

	// �Ͽ�һ������
	// id				:�ͻ���ID
	virtual bool Disconnect( EXEC_ID id );


private:

	HANDLE GetCompletionPort(){ return m_hCompletionPort; }

	SocketObject* GetSocketObject(EXEC_ID id){ return m_mapSockObj[id]; }

protected:

	// �߳���ڵ��ú���
	static DWORD WINAPI _ThreadProcCB( void *pData );

	// ���յ���Ϣʱ���߼�
	bool	OnRecvCompleted(EXEC_ID id, MOVERLAPPED *pOL);

private:

	std::map< EXEC_ID, SocketObject* >		m_mapSockObj;	// ���ӵ��׽��� first exeid		second SocketObject

	std::map< DWORD, HANDLE >		m_mapThread;			// �߳��б� first Thread ID		second Thread Handle

	SOCKET							m_sock;					// ����socket

	HANDLE							m_hCompletionPort;		// ��ɶ˿�

	UnusedID*						m_pUnusedID;			// ID������

	NetRecvCallBack*					m_pfnRecvCB;				// ���յ�������Ϣʱ�Ļص�����
	NetAcceptCallBack*					m_pfnAcptCB;				// �ɹ����յ��ͻ�������ʱ�Ļص�����

};

};


#endif