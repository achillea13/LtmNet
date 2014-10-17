#include "UnusedID.h"

namespace ltm_net{

UnusedID::UnusedID( int iMaxID )
{

	// 初始化未使用列表
	for ( int i = 1; i <= iMaxID; i++ )
	{
		_queID.push( i );
	}
}

UnusedID::~UnusedID()
{
}

int UnusedID::GetUnusedID()
{
	// 加入互斥锁
	MutexLock	mLock( _mutexID );

	if ( _queID.empty() == false )
	{
		int iID = _queID.front();
		_queID.pop();

		return iID;
	}

	return 0;
}


void UnusedID::GiveBackID( int iID )
{
	// 加入互斥锁
	MutexLock	mLock( _mutexID );

	_queID.push( iID );
}

};