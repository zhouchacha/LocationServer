#include "TcpClient.h"

/*************************************TcpClient::recvdata()*********************************************/
coordinate_t TcpClient::recvdata() {
#if BURDEN_ANALY
	time_t recvtime = time(NULL);
	time_t sendtime;
#endif
	Value ResponseData;
	Reader reader;
	coordinate_t recvcoordinate;
	char recvdata[MAX_LINE];
	if (0 != (recv(this->clientsock, recvdata, MAX_LINE, 0))) {
		string recvmsg = recvdata;
		int typecode;
		//收到的是定位请求报文的应答包
		if (reader.parse(recvmsg, ResponseData) && 1540 == (typecode =
				ResponseData["typecode"].asInt())) {
			//收集时间戳，并作记录
#if BURDEN_ANALY
			sendtime = (unsigned long) ntohl(
					ResponseData["timestamp"].asDouble());
			cout << "Client(" << getpid() << "):recv packet -- timestamp:"
					<< sendtime << " at the time " << recvtime << endl;
			this->responsetime.push_back((unsigned int) (recvtime - sendtime));
#endif
			//收集应答包中的坐标，并返回
			int x_coordinate = ResponseData["x"].asInt();
			int y_coordinate = ResponseData["y"].asInt();
			recvcoordinate = make_pair(x_coordinate, y_coordinate);
			//cout << "x:" << x_coordinate << " y:" << y_coordinate << endl;
		}
	}
	return recvcoordinate;
}

/*************************************TcpClient::senddata*********************************************/
void TcpClient::senddata(string msg) {
	short int datalen = msg.length();
	datalen = htons(datalen);
	send(this->clientsock, (char*) &datalen, 2, 0);
	send(this->clientsock, msg.c_str(), msg.length(), 0);
}

/*************************************TcpClient::clientMsg*********************************************/
string TcpClient::clientMsg() {
	Value RequestLocation;
	FastWriter writer;
	RequestLocation["typecode"] = 1040;
	RequestLocation["rssi"] =
			"74:ea:3a:26:10:86,-92.0;20:dc:e6:6b:36:10,-86.0;30:49:3b:09:68:25,-67.14492753623189;50:bd:5f:06:72:84,-85.95;00:87:36:07:ef:4d,-79.70689655172414;30:49:3b:09:68:27,-52.971014492753625;d8:15:0d:38:5b:4e,-62.68115942028985;30:49:3b:09:6b:49,-78.08695652173913;30:49:3b:09:6a:45,-85.0925925925926;08:10:74:67:63:1a,-92.16666666666667;38:83:45:96:c7:6c,-64.4927536231884;44:2b:03:8b:80:c8,-55.82608695652174;38:83:45:47:51:84,-87.0754716981132;c8:3a:35:3f:4d:98,-84.32075471698113;74:ea:3a:2f:6d:0a,-64.7536231884058;e4:d3:32:e3:fc:38,-87.66666666666667;30:49:3b:09:6a:4f,-73.05084745762711;e4:d3:32:eb:f9:f2,-90.0;b0:48:7a:5d:f3:28,-35.57971014492754;5c:63:bf:37:27:6c,-75.08823529411765;e0:05:c5:b3:d3:9e,-88.26666666666667;30:49:3b:09:6a:4b,-87.85714285714286;78:a1:06:fd:41:84,-86.37037037037037;30:49:3b:09:6a:59,-89.54545454545455;20:dc:e6:88:5b:5e,-89.57142857142857;20:dc:e6:6d:13:0e,-77.61764705882354;00:23:cd:83:7d:a0,-90.0;30:49:3b:09:6b:4b,-85.91304347826087";
#if BURDEN_ANALY
	RequestLocation["timestamp"] = htonl(time(NULL));
#endif
	//cout << RequestLocation << endl;
	return writer.write(RequestLocation);
}
/*************************************TcpClient::setstatus*********************************************/
void TcpClient::setstatus(bool value) {
	pthread_mutex_trylock(&(this->mutex));
	this->statue = value;
	pthread_mutex_unlock(&(this->mutex));
}
/*************************************TcpClient::getstatus*********************************************/
bool TcpClient::getstatus() {
	pthread_mutex_trylock(&(this->mutex));
	bool a = statue;
	pthread_mutex_unlock(&(this->mutex));
	return a;
}
/*************************************TcpClient::clientConnect*********************************************/
int TcpClient::clientConnect(sockaddr_in addr, int len, int nsec) {

	int ret;
	sigset_t nset;
	struct sigaction nsig;
	nsig.sa_handler = conntimeout;
	sigemptyset(&nset);
	nsig.sa_mask = nset;
	nsig.sa_flags = 0;
	nsig.sa_flags |= SA_INTERRUPT;
	sigaction(SIGALRM, &nsig, NULL);

	//用于产生SIGALRM信号，提前结束系统规定的75s阻塞，变为自己定义超时值
	if (alarm(nsec)) {
		perror("Client:alarm error!");
	}
	ret = connect(this->clientsock, (sockaddr *) &addr, len);
	//cout << "connect ret = " << ret << endl;
	alarm(0); //cancel the clock
	return ret;
}
/*************************************TcpClient::writeBypipe*********************************************/
bool TcpClient::writeBypipe(void) {
	char sendbuff[MAX_LINE];
	memset(sendbuff, 0, MAX_LINE);
	vector<coordinate_t>::iterator iter;
	int i;
	//cout << "Child is vector's size = " << coordinate.size() << endl;
#if BURDEN_ANALY
	vector<unsigned int>::iterator iter2;
	for (iter = this->coordinate.begin(), iter2 = this->responsetime.begin(), i =
			0; iter != coordinate.end(); iter++, iter2++, i++) {
		debug("%7d%7d%5u\n", (*iter).first,(*iter).second, (*iter2));
		sprintf(sendbuff + i * 20, "%7d%7d%5u\n", (*iter).first, (*iter).second,
				(*iter2));
	}
#else
	for(iter=this->coordinate.begin(),i=0;iter!=coordinate.end();iter++,i++)
	{
		debug("Child:x= %d y = %d \n",(*iter).first,(*iter).second);
		sprintf(sendbuff+i*15,"%7d%7d\n",(*iter).first,(*iter).second);
	}
#endif

	write(this->pipewrite, sendbuff, sizeof(sendbuff));
	cout<<"Client["<<getpid()<<"]: write pipe end!"<<endl;
	return 0;
}
/*************************************TcpClient::clientInit*********************************************/
void TcpClient::clientInit(sockaddr_in addr, int pipereadId) {
	pthread_mutex_init(&(this->mutex), NULL); //初始化互斥锁
	this->setstatus(false); //初始后状态字
	this->pipewrite = pipereadId; //设置写管道ID
	debug("Child:pipewriteID:%d child's process pid:%d\n" ,this->pipewrite,getpid());

	this->clientsock = socket(AF_INET, SOCK_STREAM, 0);
	if (this->clientsock == -1) {
		cout << "create client error" << endl;
	}

	if (0 == clientConnect(addr, sizeof(sockaddr_in), CONNECTTIMEOUT)) {
		//正常连接，则创建发送线程
		pthread_create(&(this->tid), NULL, PerTransFun, (void *) this); //创建周期性发数据的线程
	} else {
		//处理硬错误，错误代码111，此时connect不会阻塞到SIGALRM信号的发生
		cout << "Client(" << getpid() << "):connect timeout within given time";
		perror("");
		exit(-1);
	}
	//创建select来接收到来的包，并调用函数进行分析
	fd_set rset;
	FD_ZERO(&rset);
	struct timeval tt;
	tt.tv_sec = 1;
	tt.tv_usec = 0;

	while (1) {
		FD_SET(this->clientsock, &rset);
		select((this->clientsock) + 1, &rset, NULL, NULL, &tt);
		if (this->getstatus() && FD_ISSET(this->clientsock,&rset)) {
			this->coordinate.push_back(recvdata());
		} else if (!this->getstatus()) {
			break;
		}
	}
	pthread_join(this->tid, NULL);
	debug("Child:init end child's process pid:%d\n" , getpid());
}
/*************************************(*TcpClient::PerTransFun)*********************************************/
void *TcpClient::PerTransFun(void *object) {

	TcpClient *pt = (TcpClient *) object;
	pt->setstatus(true);
	time_t startTime = time(NULL);
	time_t endTime = time(NULL);

	//创建需要发送的消息sendMsg

	cout << "Client(" << getpid() << "):Packet send begin at "
			<< asctime(localtime(&endTime));

	while ((time(NULL) - startTime) <= TESTDURATION) {
		pt->period.tv_sec = PERIODSENDMSGSEC;
		pt->period.tv_usec = PERIODSENDMSGUSEC;
		int status = select(0, NULL, NULL, NULL, &(pt->period));
		if (0 == status) {
			pt->senddata(pt->clientMsg());

		}
	}

	//cout << "Client(" << getpid() << "):go into sleep! "<<endl;
	//sleep(KEEPALIVEDURATION);

	pt->setstatus(false);
	endTime = time(NULL);
	cout << "Client(" << getpid() << "):Packet send end in "
			<< asctime(localtime(&endTime));
	pthread_exit(0);
}

/*************************************connect_alarm*********************************************/
static void conntimeout(int signo) {
	return;
}
