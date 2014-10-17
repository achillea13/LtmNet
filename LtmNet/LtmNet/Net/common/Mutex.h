#ifndef __LTM_NET_MUTEX__
#define __LTM_NET_MUTEX__

namespace ltm_net
{

#ifdef _WIN32

#include <Windows.h>

class Mutex
{

protected:
	CRITICAL_SECTION	_csObj;			// for windows

public:

	Mutex()
	{
		InitializeCriticalSection( &_csObj );
	}

	virtual ~Mutex()
	{
		DeleteCriticalSection( &_csObj );
	}

	// 锁定
	void Lock()
	{
		EnterCriticalSection( &_csObj );
	}

	// 解锁
	void UnLock()
	{
		LeaveCriticalSection( &_csObj );
	}
};



class MutexLock
{
protected:
	Mutex		*_pMutex;

public:
	MutexLock( Mutex &mutex )
	{
		_pMutex	= &mutex;
		_pMutex->Lock();
	}

	virtual ~MutexLock()
	{
		_pMutex->UnLock();
	}

};


// helper class for lock
class Locker
{
public:

	Locker( CRITICAL_SECTION& cs ) : _pcs( &cs )
	{
		Lock();
	}

	~Locker()
	{
		if ( _pcs )
			Unlock();
	}

	void Lock()
	{
		EnterCriticalSection( _pcs );
	}

	void Unlock()
	{
		LeaveCriticalSection( _pcs );
		_pcs = NULL;
	}


protected:

	CRITICAL_SECTION*	_pcs;

};

#endif	// if _WIN32 end


class FlagLock
{
protected:
	bool	_bFlag;

public:

	FlagLock(){ Reset(); }

	// 试图锁定
	bool TryLock()
	{
		if ( !_bFlag )
		{
			_bFlag = true;
			return true;
		}

		return false;
	}

	// 解锁
	void UnLock(){ _bFlag = false; }

	// 重置为未锁定状态
	void Reset(){ UnLock(); }

	// 是否已经锁定
	bool IsLock(){ return _bFlag; }


};

};

#endif