#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <TcpUser.h>
#include <MsgQueue.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <event.h>
#include <unistd.h>

#include <string>
using std::string;

#include <memory>
using std::shared_ptr;


/**
 * 该类表示一个服务器对象。
 */
class TcpServer{
public:
	friend class TcpUser;
	explicit TcpServer(MsgQueue &msgQueue):msgQueue(msgQueue){};

	/**
	 * 初始化操作。包括：
	 * 1、利用输入参数初始化服务器套接字的IP和端口号。
	 * 2、初始化基事件，基事件是用来注册事件的对象，它会不断的检查注册
	 * 到其中的事件的状态。但不开启基事件，即基事件还不会立即开始检测被
	 * 注册事件的状态，基事件的开启在Listen函数中。
	 * 3、把服务器套接字的可读事件（即客户端的connect引发的事件）注册到基事件。	
	 * @param  ip   服务器的IP
	 * @param  port 服务器的端口号
	 * @return      初始化成功返回 0.初始化过程中如果出现错误则返回-1
	 * 出错情况可能发生在：创建服务器套接字、绑定套接字到套接字地址
	 * 创建基事件、创建套接字读事件
	 */
	int Init(string ip, int port);


	/**
	 * 利用输入参数执行服务器套接字的listen操作，并开启基事件。	
	 * @param backlog 服务器套接字监听队列的长度
	 */
	void Listen(int backlog);


	/**
	 * 被TcpUser::OnRead函数调用。断开一条连接，删除该TcpUser 对象在users中的智能指针
	 * @param ptr 调用者的指针
	 */
	void OffConnection(TcpUser *ptcpUser);


	/**
	 * 被TcpUser::OnRead函数调用。删除users中的所有对象。
	 * 关闭可读事件，关闭基事件。
	 */
	void QuitServer(void);


	/**
	 * 把参数事件evt加入到基事件base当中
	 * @param  evt 要加入的事件
	 * @return     -1表示失败，0表示成功
	 */
	// int AddEvent(struct event* evt);
	

	/**
	 * 服务器套接字读事件的回调函数，在套接字读事件发生时被调用。
	 * 接受连接，为新连接创建套接字和TcpUser对象，并初始化TcpUser对象
	 * @param sock 	该事件的套接字
	 * @param what 该事件
	 * @param arg  传入参数
	 */
	static void OnAccept(int sock,short what, void *arg);


	/**
	 * 析构函数。1、关闭服务器套接字。2、释放服务器套接字读事件。
	 * 3、释放服务器基事件。
	 */
	~TcpServer()
	{
		close(tcpServer);
	}

private: 
	int tcpServer; 
	struct sockaddr_in SockAddr;

	//连接事件
	struct event *serverEvt;

	//记录连接上的用户
	std::vector<shared_ptr<TcpUser> > users;

	//基事件
	struct event_base* base;

	//请求队列
	MsgQueue &msgQueue;
};

#endif