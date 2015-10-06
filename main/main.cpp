#include <DBConnpool.h>

#include <TcpServer.h>
#include <Location.h>
#include <Guide.h>
#include <MsgQueue.h>
#include <define.h>
#include <iostream>
using namespace std;

#define BACKLOG 2

ConnPool *pool;

int main(void)
{
    // pool = ConnPool::GetInstance();
	MsgQueue msgQueue;
	Guide guide(&msgQueue);
	Location location(&msgQueue);	
	TcpServer server(msgQueue);
	if(server.Init(IP, PORT) != 0)
		return 0;
	server.Listen(BACKLOG);
}
