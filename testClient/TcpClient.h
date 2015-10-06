#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H

#include <define.h>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <cstring>


using namespace std;
using namespace Json;

#define TESTDURATION 4		//设置客户端发送报文的时间-秒
#define CLIENTNUM 50		//设置需要模拟的客户端数目
#define PERIODSENDMSGSEC 1	//设置报文发送的时间间隔-秒
#define PERIODSENDMSGUSEC 0//设置报文发送的时间间隔-微秒
#define CONNECTTIMEOUT   5	//设置连接服务器的超时时间-秒
//#define KEEPALIVEDURATION	10		//设置客户端发送完报文后的等待时间-秒
   
#define MAX_LINE (20*10*TESTDURATION)//根据发送的时长来定义缓冲的大小，防止"stack smashing detected"

typedef pair<int, int> coordinate_t;

class TcpClient {
private:
	vector<coordinate_t> coordinate;							//用于收集返回的定位坐标
#if BURDEN_ANALY
	vector<unsigned int> responsetime;							//用于收集定位请求的响应时间
#endif
	//vector<float> responsetime;
	int clientsock;												//TcpClient的套接字
	//char recvdata[MAX_LINE];
	//string sendMsg;											//TcpClient发送的消息
	pthread_t tid;												//发送线程的线程ID
	pthread_mutex_t mutex;										//设置statue的互斥锁
	struct timeval period;
	bool statue;												//发送线程用于通知主线程结束发送，主线程回收发送线程的存储资源
	int pipewrite;

public:
	TcpClient(){};
	~TcpClient(){
		cout<<"TcpClient sock close!!!"<<endl;
	}
	static void *PerTransFun(void *object);						//发送线程的入口函数
	coordinate_t recvdata(void);								//接收返回的报文处理函数
	void senddata(string msg);									//发送消息的函数
	string clientMsg(void);										//构造TcpClient发送的string消息-由senddata调用
	void clientInit(sockaddr_in addr,int pipereadId);			//TcpClient的主线程
	int clientConnect(sockaddr_in addr, int len,int nsec);		//TcpClient连接服务器，添加连接超时功能
	void setstatus(bool value);									//设置状态字-stauts
	bool getstatus(void);										//获取状态字-stauts
	bool writeBypipe(void);										//将需要的内容通过管道传给父进程
};

static void conntimeout(int signo);								//通过信号处理函数来提前结束阻塞状态下的connect函数

#endif
