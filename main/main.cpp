#include <TcpServer.h>
#include <Location.h>
#include <Guide.h>
#include <MsgQueue.h>
#include <define.h>

#define BACKLOG 2


int main(void)
{
	MsgQueue msgQueue;
	Guide guide(&msgQueue);
	Location location(&msgQueue);	
	location.init();
	TcpServer server(msgQueue);
	if(server.Init(IP, PORT) != 0)
		return 0;
	server.Listen(BACKLOG);
}
