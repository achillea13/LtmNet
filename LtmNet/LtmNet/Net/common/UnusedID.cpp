#include "UnusedID.h"

namespace ltm_net{

UnusedID::UnusedID( int iMaxID )
{

	// ��ʼ��δʹ���б�
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
	// ���뻥����
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
	// ���뻥����
	MutexLock	mLock( _mutexID );

	_queID.push( iID );
}

};