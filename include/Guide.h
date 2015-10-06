#ifndef _ASTAR_H_
#define _ASTAR_H_

#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <MsgQueue.h>

using namespace std;

#define INFI_MAX 65535
#define T 20
#define W0 0.2
#define NODENUM 31
#define EDGENUM 66

const int M = 100;
const int N = 100;

struct node{     // µØÍ¼½Úµã
	string nodename;
	double position_x;
	double position_y;
	//int position_dp;
	int m_index;
};

struct edge{
	int edgeid;
	double edgeweightid;  //È¨ÖØ
	int frontnodeid;
	int lastnodeid;
};

typedef struct GraphMatrix   //µØÍ¼¾ØÕó
{
	int matrix[N][M];
	int node_num;
	int edge_num;
}Graph;

struct starnode{
	int nodeid;
	double positionx;
	double positiony;
	double g;
	double h;
	double f;
	starnode* parent;
	starnode(int _nodeid,int _positionx,int _positiony):nodeid(_nodeid),positionx(_positionx),positiony(_positiony){
		g = 0.0;
		h = 0.0;
		f = 0.0;
		parent = NULL;
	}
};

class Guide
{
public:
	Guide(MsgQueue* msgQueue);
	~Guide();
	void initGraph(Graph& g);
	void deleteInOpenVec(int nodeid);
	starnode* isInOpenVec(int nodeid);
	starnode* isInCloseVec(int nodeid);
	void deleteVec();
	bool check(node Anode, starnode* Bnode, starnode* Cnode);
	double distanceFromA2B(starnode* a, starnode* b);
	starnode* findMinNode2EndInOpen(starnode* eNode);
	bool checkPath(node curnode, starnode* parentNode,starnode* eNode);
	list<starnode*> findPath(starnode* eNode);
	list<starnode*> searchpath(int startpoint, int endpoint);
	void loadMap();
//	void loadMap2();
	int getIndexStart(double point_x, double point_y);
	static void* guide(void* arg);

	int test(void);
	void record(void);

private:
	MsgQueue *msgQueue;

	int startPoint_index;   //开始节点
	int endPoint_index;     //目的节点

	map<int, int> parent;  //父节点，及可扩展节点
	map<int, int> dist;   //距离

	node newnode[NODENUM];  // 创建节点
	edge newedge[EDGENUM]; //边数

	Graph g;

	map<int, node> nodeMap;  // 存放节点信息
	map<int, edge> edgeMap; // 存放边信息

	vector <starnode*> v_open; //open 表
	vector <starnode*> v_close; // close 表


	/**
	 * 用来测试导引时间，guideDuring记录每次从请求队列里取出请求时的时间。
	 *receiveBufSize记录此时的请求队列里数据的个数。 
	 */
	vector<struct timeval> guideDuring;
	vector<int> receiveBufSize;

};
#endif // _ASTAR_H_
