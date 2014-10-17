#include "Agent.h"


#ifdef _WIN32
#include "impl\win\iocp\Server.h"
#include "impl\win\iocp\Client.h"
#endif

namespace ltm_net{

AgentServer::AgentServer()
{

	_pNetServer = NULL;
	
}

AgentServer::~AgentServer()
{
	SAFE_DELETE(_pNetServer);
}

bool AgentServer::Init( int iMaxClient, unsigned short usPort, NetRecvCallBack fnRecvCB, NetAcceptCallBack fnAcptCB, int iThreadNum )
{

#ifdef _WIN32
	_pNetServer = new Server();
	_pNetServer->Startup( iMaxClient, usPort, fnRecvCB, fnAcptCB, iThreadNum );
#endif



	return true;
}


bool AgentServer::Send( std::vector<EXEC_ID> vecId, char* pBuf, int iLen )
{

#ifdef _WIN32

	if ( _pNetServer != NULL )
		return _pNetServer->SendData( vecId, pBuf, iLen );
	else 
		return false;

#endif
	
}



AgentClient::AgentClient()
{
	_pNetClient = NULL;
}

AgentClient::~AgentClient()
{
	SAFE_DELETE( _pNetClient );
}

bool AgentClient::Init( const char* strIP, unsigned short usPort, NetRecvCallBack fnRecvCB )
{
#ifdef _WIN32
	_pNetClient = new Client();
	return _pNetClient->Startup( strIP, usPort, fnRecvCB );
#endif
	

	return false;
}

bool AgentClient::Send( char* pBuf, int iLen )
{
#ifdef _WIN32
	if ( _pNetClient == NULL )
		return false;
	else
		return _pNetClient->SendData( pBuf, iLen );
#endif

	return false;
}

}