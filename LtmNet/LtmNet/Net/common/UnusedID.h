#ifndef __LTM_NET_UNUSEDID__
#define __LTM_NET_UNUSEDID__

#include <queue>
#include "Mutex.h"

namespace ltm_net
{

class UnusedID
{
public:
	UnusedID( int iMaxID );
	virtual ~UnusedID();

	// ��ȡһ��δʹ�õ�ID
	int GetUnusedID();

	// �黹һ��ID
	void GiveBackID( int iID );

protected:

	std::queue< int >	_queID;	
	Mutex				_mutexID;

};

};

#endif