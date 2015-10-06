#include "TcpClient.h"

/**
 * 指定生成N个客户端，每个客户端使用select监听套接字的接收数据；
 * 另有发送线程，每隔SendDuration s发送一次定位请求，持续发送的时间为TestDuration结束；
 * 计算每次的定位请求相应时间，并作记录；
 * 结束时候，各个客户端将自己的响应时间统一记录到filename.txt中（使用匿名管道实现，另一种可以尝试用FIFO/消息队列/共享内存来实现）；
 * 添加了客户端connect超时功能（信号处理函数sigaction和不自重启设置来实现，还可以借用select和非阻塞实现，另一种是借用系统定义的75s来实现）；
 * 修改了日志文件的命名规则，添加仿真时间以及一些参数，并修改了存放日志文件的位置；
 * 添加了子进程结束时向父进程发送信号的通信机制,使得父进程能寿终正寝（当前是通过读管道有效的次数来实现，当次数等于模拟的客户端数目时，主进程结束）；
 * 僵尸进程的避免：	1>正常情况下 client的父进程会结束，不会有僵尸进程存在；
 * 					2>特殊情况下(connect超时，导致父进程正常终止的条件无法满足)，因此添加信号机制，避免因模拟客户端的父进程意外没有终止而产生过多的僵尸进程；
 *
 ****/
static void avoidzombie(int signo) {
	static int i = 1;
	cout << "i = " << i <<" Zombie processes has been avoid!"<< endl;
	i++;
	return;
}

int main(void) {
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = inet_addr(IP);

	//避免产生僵尸进程
	sigset_t newset;
	struct sigaction new_sig, old_set;
	new_sig.sa_handler = avoidzombie;
	sigemptyset(&newset);
	new_sig.sa_mask = newset;
	new_sig.sa_flags = 0;
	new_sig.sa_flags |= SA_NOCLDWAIT;
	//new_sig.sa_flags |= SA_NOCLDSTOP;
	sigaction(SIGCHLD, &new_sig, &old_set);

	int totalpipe[CLIENTNUM];
	int pipeId[2];

	//启动指定的模拟客户端数目
	int clientnum = 0;
	while (clientnum < CLIENTNUM) {
		if (0 != pipe(pipeId)) {
			cout << "PARENT:Pipe creates error!" << endl;
		}
		if (0 == fork()) {
			close(pipeId[0]);
			int pipewrite = pipeId[1];

			TcpClient *user = new TcpClient();
			user->clientInit(serveraddr, pipewrite);
			user->writeBypipe();
			delete user;
			exit(0);
		}
		close(pipeId[1]);
		totalpipe[clientnum] = pipeId[0];
		clientnum++;
	}

	//
	//通过管道将数据传递进父进程，并作日志记录
	debug("Client(%d): I am the main thread!",getpid());
	cout << "Client(" << getpid() << "): I am the main thread!" << endl;
	int clientrecv = 0;
	char buffer[MAX_LINE];
	//char *filename=(char *)calloc(100,1);		//实现开辟和清零
	char *filename = (char *) malloc(100); //实现开辟，没有清零，但是"***"会自动添加'\0'
	time_t simulationTime;
	ofstream outfile;

	simulationTime = time(NULL);

	//filename's meanning:模拟的客户端数目+客户端运行的时间长度(s)+定位请求发送的时间+间隔仿真的日期和时间.txt
	sprintf(filename, "./data/%3d_%3d_%5.3f_%s.txt",CLIENTNUM, TESTDURATION,
		((float) PERIODSENDMSGSEC + PERIODSENDMSGUSEC / 1000000),
		asctime(localtime(&simulationTime)));
	outfile.open(filename, fstream::app | fstream::out);
	debug("File descriptor:%0x",outfile);

	while (1) {
		clientnum = 0;
		while (clientnum < CLIENTNUM) {
			int ret = read(totalpipe[clientnum], buffer, MAX_LINE);
			if (ret > 0) {
				outfile << "ClientID:" << clientnum + 1 << endl;
				outfile << buffer;
				//cout << buffer << endl;
				clientrecv++;
				//cout<<clientrecv<<endl;
			}
			clientnum++;
		}
		if (clientrecv == CLIENTNUM)
			break;
	}
	outfile.close();
	return 0;
}
