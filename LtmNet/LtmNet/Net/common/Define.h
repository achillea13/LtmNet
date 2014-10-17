#ifndef __LTM_NET_DEFINE__
#define __LTM_NET_DEFINE__

#include <time.h>
#include <math.h>

#include <list>
#include <vector>
#include <stack>
#include <map>
#include <queue>
#include <string>
#include <algorithm>

namespace ltm_net
{


#ifndef SAFE_DELETE
#define SAFE_DELETE(p)	{ if(p){ delete p; p = 0; } }
#endif


#ifndef SAFE_CLOSE_HANDLE
#define SAFE_CLOSE_HANDLE(p)	{ if(p){CloseHandle(p); p = 0; } }
#endif


#ifndef ARRAY_COUNT
#define ARRAY_COUNT(a)	(sizeof(a) / sizeof(a[0]))
#endif

#define WSABUFF_LEN_LMT 8 * 1024			// �������ݴ��䳤������



#define EXEC_ID	unsigned long
enum AcceptErrorCode
{
	AEC_OK			= 0,			// ���ӳɹ�
	AEC_UNKNOW,						// δ֪����
	AEC_NUM_LMT,					// �ﵽ��������
	AEC_COMPORT_FAIL,				// ��ɶ˿ڴ���ʧ��		
	AEC_SOCKOBJ_FAIL,				// socket object����ʧ��
	AEC_WSARECV_PENDING,			// Ͷ��ʧ��
};

typedef void (NetRecvCallBack) (int, char*, int);					// exec_id, buf, len
typedef void (NetAcceptCallBack) ( EXEC_ID, int, char*, int );		// exec_id, AcceptErrorCode, buf, len

}

#endif