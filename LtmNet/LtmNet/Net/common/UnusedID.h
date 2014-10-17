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

	// 获取一个未使用的ID
	int GetUnusedID();

	// 归还一个ID
	void GiveBackID( int iID );

protected:

	std::queue< int >	_queID;	
	Mutex				_mutexID;

};

};

#endif