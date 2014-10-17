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

	// ����������
	// iMaxClient		:���������
	// iThreadNum		:�����߳���
	// usPort			:�˿�
	// fnRecvCB			:���յ��������ݰ�ʱ�Ļص�����
	virtual bool Startup( int iMaxClient, unsigned short usPort, NetRecvCallBack fnRecvCB, NetAcceptCallBack fnAcptCB, int iThreadNum ) = 0;

	// ����һ���������ݰ�
	// vecID			:Ҫ���͵��Ŀͻ��˼���
	// pBuf				:���͵�����
	// iLen				:�������ݳ���
	virtual bool SendData( std::vector<EXEC_ID> vecId, char* pBuf, int iLen ) = 0;

	// �Ͽ�һ������
	// id				:�ͻ���ID
	virtual bool Disconnect( EXEC_ID id ) = 0;
};



class ClientInterface
{
public:
	ClientInterface(){};
	virtual ~ClientInterface(){};

	// �����ͻ��ˣ����ӷ�����
	// strIP			:ip
	// usPort			:�˿�
	virtual bool Startup( const char* strIP, unsigned short usPort, NetRecvCallBack fnRecvCB ) = 0;

	// ����һ���������ݰ�
	// pBuf				:����
	// iLen				:����
	virtual bool SendData( char* pBuf, int iLen ) = 0;

};

};

#endif