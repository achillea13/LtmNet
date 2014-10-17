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

	// ��ʼ��������
	// iMaxClient		:���������
	// iThreadNum		:�����߳���
	// usPort			:�˿�
	// fnRecvCB			:�յ���Ϣʱ�Ļص�����
	bool Init( int iMaxClient, unsigned short usPort, NetRecvCallBack fnRecvCB, NetAcceptCallBack fnAcptCB, int iThreadNum = 1 );

	// ����һ���������ݰ�
	// vecID			:Ҫ���͵��Ŀͻ��˼���
	// pBuf				:���͵�����
	// iLen				:�������ݳ���
	bool Send( std::vector<EXEC_ID> vecId, char* pBuf, int iLen );

protected:

	ServerInterface* _pNetServer;

};

class AgentClient
{
public:
	AgentClient();
	virtual ~AgentClient();

	// ��ʼ���ͻ��ˣ����ӷ�����
	// strIP			:������IP
	// usPort			:�������˿�
	// fnRecvCB			:���յ���Ϣʱ�Ļص�����
	bool Init( const char* strIP, unsigned short usPort, NetRecvCallBack fnRecvCB );

	// ����һ�����ݰ���������
	bool Send( char* pBuf, int iLen );

protected:

	ClientInterface* _pNetClient;
};


}


#endif