// LtmNet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include "Agent.h"

#include <Windows.h>

ltm_net::AgentServer as;
void RecvCallBack (int exeid, char* buf, int len)
{
	printf( "收到消息 : %s\n", buf );
	std::vector< unsigned long > vID;
	vID.push_back(1);
	char buff[128] = {0};
	sprintf( buff, "客户端回执消息" );
	as.Send( vID, buff, strlen(buff)  );
}

void AcptCallBack (EXEC_ID exeid, int errorid, char* buf, int len)
{
	if ( errorid > ltm_net::AEC_OK )
	{
		printf("连接失败， 错误代码:%d\n", errorid);
	}
	else
		printf("连接成功，exeid = %d\n", exeid );

}

int _tmain(int argc, _TCHAR* argv[])
{


	as.Init( 5, 3125, RecvCallBack, AcptCallBack );




	getchar();
	return 0;
}

/*
void RecvCallBack (int exeid, char* buf, int len)
{
	printf( "收到消息 : %s\n", buf );
}

int _tmain(int argc, _TCHAR* argv[])
{

	ltm_net::AgentClient ac;
	ac.Init("127.0.0.1" , 3125, RecvCallBack );

	int i = 1;
	while ( 1) 
	{
	
		char buff[128] = {0};
		sprintf(buff, "msg : %d", i );
		ac.Send( buff, strlen(buff) );
		printf( "发送消息：%s\n", buff );

		Sleep(3000);
	}



	getchar();
	return 0;
}*/