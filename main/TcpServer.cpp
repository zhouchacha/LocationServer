#include <TcpServer.h>
#include <TcpUser.h>
#include <define.h>

#include <memory>

int TcpServer::Init(string ip, int port)
{
	if((tcpServer = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		debug("create tcpServer error");
		return -1;
	}
	int yes = 1;
	setsockopt(tcpServer, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(port);
	SockAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	if(bind(tcpServer, (struct sockaddr*)&SockAddr,sizeof(struct sockaddr)) == -1)
	{
		debug("bind tcpServer to SockAddr error");
		return -1;
	}	

	if((base = event_base_new()) == NULL)
	{
		debug("create event base error");
		return -1;
	}
	if((serverEvt = event_new(base, tcpServer, EV_READ|EV_PERSIST, OnAccept,this)) == NULL)
	{
		debug("create severEvt error");
		return -1;
	}
	event_add(serverEvt, NULL);
	debug("init tcpServer success");
	return 0;
}

void TcpServer::Listen(int backlog)
{
	listen(tcpServer,backlog);
	event_base_dispatch(base);
}

void TcpServer::OffConnection(TcpUser *ptcpUser)
{
	for(auto user = users.begin(); user != users.end(); user++)
	{
		if((*user).get() == ptcpUser)
		{
			users.erase(user);
			debug("delete an user");
			return;
		} 
	}
}

void TcpServer::QuitServer(void)
{
	for(auto user = users.begin(); user != users.end(); user++)
		*user=NULL;

	event_free(serverEvt);
	event_base_loopbreak(base);
	debug("quit server");
}

// void TcpServer::AddEvent(strcut event* evt)
// {

// }

void TcpServer::OnAccept(int sock, short what, void *arg)
{
	debug("accept a client");
	TcpServer *ptcpServer = (TcpServer*)arg;
	int client = accept(sock,(struct sockaddr*)NULL, NULL);
	shared_ptr<TcpUser> user = std::make_shared<TcpUser>(client,ptcpServer);
	ptcpServer->users.push_back(user);
	user->Init();
}