#ifndef TCPUSER_H
#define TCPUSER_H

// #include <unistd.h>
#include <event.h>
#include <string.h>
#include <MsgQueue.h>
#include <queue>

#include <string>
using std::string;

class TcpServer;

/**
 * 该类的对象对应一个客户端。
 */
class TcpUser
{
public:
	TcpUser(int fd,TcpServer *tcpServer);

	/**
	 * 初始化TcpUser对象。为套接字创建读事件和写事件。
	 * @return      初始化成功返回 0，失败返回-1，失败的情况可能是：创
	 * 建读、写事件失败。
	 */
	int Init();

	/**
	 * 设置用户名
	 * @param name 用户名
	 */
	void SetUserName(string name);


	/**
	 * 设置用户电话号码
	 * @param name电话号码
	 */
	void SetUserTel(string tel);


	/**
	 * 获取ID
	 * @return  ID
	 */
	int GetId(void) const;


	/**
	 * 处理收到的数据，根据不同的数据类型进行不同的处理
	 * @param data 收到的数据
	 */
	void DataHandle(string &data);

	/**
	 * 描述：如果写事件没有绑定则绑定。
	 */
	void PendingWriteEvt();


	/**
	 * 从套接字接收数据，进行预处理。目前数据传输协议采用：长度+数据 的形式。
	 * 在该函数里要进行分离长度与数据并判断接收是否成功。
	 * @param  str 引用参数，用来传出接收到的数据。
	 * @return     -1表示失败 0表示成功
	 */
	int MyRead(string &str) const;


	/**
	 * 套接字读事件的回到函数，可读时被调用。	
	 * @param sock 套接字
	 * @param what 事件	
	 * @param arg  传入的参数
	 */
	static void OnRead(int sock, short what, void *arg);


	/**
	 * 把要发送的数据封装成 长度+数据的形式。
	 * @param  str 要发送的数据
	 * @return     -1表示失败 0表示成功
	 */
	int MyWrite(const string &str) const;


	/**
	 * 套接字写事件的回掉函数，可写事调用
	 * @param sock 套接字
	 * @param what 事件	
	 * @param arg  传入的参数
	 */	
	static void OnWrite(int sock, short what, void *arg);


	/**
	 * 释放相关资源。包括，删除读写事件，释放读写事件
	 * 和释放定位、导引以及信息队列相对应的锁
	 */	
	~TcpUser();



private:
	static int onLineNum;
	int id;

	string tel;		//用户电话号码
	string username;	//用户名
	
	int sock_fd;				//该用户套接字
	struct event *readEvt;		//读事件
	struct event *writeEvt;		//写事件
	
	MsgQueue &msgQueue;		//请求队列
	TcpServer *tcpServer;		

	std::queue<Json::Value> sendMsg;

	/**
	 * 保证动态分配的数组能正常释放,在构造函数里根据出入参数的长度分配
	 * 空间，并初始化为空。在析构函数里释放空间。
	 * 注意：未考虑分配失败的情况
	 */
	class NewDataArray
	{
	public:
		explicit NewDataArray(int len)
		{ 
			data = new char[len];
			memset(data,'\0',len);
		}
		char* GetArrayPtr(void){ return data;}
		~NewDataArray(){ delete[] data;}
	private:
		char* data;
	};


};
  

#endif
