/*
修改人：胡珑怀
时间：2015/7/7
内容：把代码封装成类
*/

#include <Guide.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <define.h>

#include <sys/time.h>
#include <stdio.h>

#include <string>
using std::string;

#include <tuple>
using std::tuple;

#include <json/json.h>
using Json::FastWriter;
using Json::Value;

Guide::Guide(MsgQueue *msgQueue):msgQueue(msgQueue)
{
	initGraph(g);
	loadMap();
	pthread_t p;
	if(pthread_create(&p, NULL, guide, this) == -1)
	{
		debug("create Guide error");
	}
	else
	{
		debug("create Guide success");
	}
}

Guide::~Guide()
{
	dist.clear();
	parent.clear();

	#if TEST
	record();
	#endif
}
void Guide::initGraph(Graph& g){   // ³õÊ¼»¯µØÍ¼ -1£¬ ÊýÖµ´ú±í±ßid
	for (int i = 0; i<N; i++){
		for (int j = 0; j<M; j++){
			g.matrix[i][j] = -1;     //×¢ÒâÐ´·¨£¬MÊÇÇ°Ò»¸ö½Úµã±àºÅ£¬NÊÇºóÒ»¸ö½Úµã±àºÅ£¬¾ØÕóµÄÖµ´ú±íÁ½½ÚµãÖ®¼äµÄÈ¨ÖØ
		}
	}
	g.node_num = 0;
	g.edge_num = 0;
}

/*************open  É¾³ý½áµãid********/    //ÔõÃ´É¾³ýÄØ£¿£¿£¿£¿£¿£¿£¿£¿£¿ÐÞ¸Ä
void Guide::deleteInOpenVec(int nodeid){        //²ÎÊý nodeid ´ú±íÐÇ½Úµã£¿£¿£¿ ÔÚ±éÀúÊ±×îÐ¡µÄ£¬½«Ö®É¾³ý
	vector<starnode*>::iterator iter;
	for (iter = v_open.begin(); iter != v_open.end(); iter++)
    {
		if (nodeid == (*iter)->nodeid)
		{
			v_open.erase(iter);  //×¢ÒâÐ´·¨
			break;
		}
	}
}

starnode* Guide::isInOpenVec(int nodeid)      //Èç¹û½ÚµãÔÚopen±íÖÐ£¬·µ»ØµÄÊÇÒ»¸öÐÇ½Úµã£¨¿ÉÀ©Õ¹½Úµã£©
{
	vector<starnode*>::iterator iter;
	starnode* result = NULL;
	for (iter = v_open.begin(); iter != v_open.end(); iter++)
	{
		if (nodeid == (*iter)->nodeid)
		{
			result = *iter;
			break;
		}
	}
	return result;
}
/************close ÀïÊÇ·ñÓÐÎ»ÖÃindexµÄ½áµã******/
starnode* Guide::isInCloseVec(int nodeid)     //Èç¹ûÐÇ½ÚµãÔÚclose±íÖÐ£¬Ôò·µ»ØÆä½Úµã
{
	vector<starnode*>::iterator iter;
	starnode* result = NULL;
	for (iter = v_close.begin(); iter != v_close.end(); iter++)
	{
		if (nodeid == (*iter)->nodeid)
		{
			result = *iter;
			break;
		}
	}
	return result;
}

void Guide::deleteVec(){      //É¾³ýÇå¿Õopen ºÍclose ±í

	for (int i = 0; i < (int)v_open.size(); i++){
		delete v_open[i];
	}
	for (int i = 0; i < (int)v_close.size(); i++){
		delete v_close[i];
	}
	v_open.clear();
	v_close.clear();

}

bool Guide::check(node Anode, starnode* Bnode, starnode* Cnode) {     //²é¿´½ÚµãÊÇ·ñÔÚÕýÏò
    if ((Cnode->positionx-Anode.position_x)*(Cnode->positionx-Bnode->positionx) + (Cnode->positiony-Anode.position_y)*(Cnode->positiony-Bnode->positiony) < 0)
        return false;
    return true;
}
/*********Á½¸ö²»Í¬indexÖ®¼äÂ·³Ì´ú¼Û*********/
double Guide::distanceFromA2B(starnode* a, starnode* b){      //Á½¸öÐÇ½ÚµãÖ®¼äµÄ¾àÀë

	double distance = sqrt((double)(a->positionx - b->positionx)*(a->positionx - b->positionx)\
		+ (a->positiony - b->positiony)*(a->positiony - b->positiony));
	return distance;
}

starnode* Guide::findMinNode2EndInOpen(starnode* eNode){   //±éÀúopen±í¡£ÕÒ³öÓëeNode¾àÀë×îÐ¡µÄ½Úµã£¨g£©£¬²¢½«Ö®´Óopen±íÄÚÉ¾³ý£¬²¢·µ»ØÐÇ½Úµã  eNode´ú±í×îºó½Úµã
	int min = 65535;      //¾àÀë¶¨ÒåÎª²»¿É´ï
	starnode* result = NULL;

	int deletenodeid = -1;
	int f = 0;
	int minf = 65535;
	starnode* p;    //ÐÇ½Úµã
	for (int i = 0; i < v_open.size(); i++) {
		p = v_open[i];    //±éÀúopen±í
		f = (int)distanceFromA2B(p, eNode);   //Á½¸ö½ÚµãÖ®¼äµÄ¾àÀëh
		f += p->g;   //Ëã³ög




		if(f<minf)
		{
			minf =f;
			result = p;

		}
		deletenodeid = result->nodeid;//printf("minf,nodeid:%d,f=%d,minf=%d\n",p->nodeid,f,minf);   //ÒÑ¾­ÐÞ¸Ä
	}

	deleteInOpenVec(deletenodeid);

	//cout<<"g"<<result->m_index<<endl;
	return result;
}

bool Guide::checkPath(node curnode, starnode* parentNode,starnode* eNode){   //Ñ°Â·¡£Èç¹ûÏÖÔÚµÄ½ÚµãÔÚclose±íÖÐ£¬Ôò½áÊø£¬Èç¹û²»ÔÚ
	//printf("enter checkPath\n");

	int curnodeid = curnode.m_index;
	if(isInCloseVec(curnodeid)){
		return false;
	}
   if(isInOpenVec(curnodeid)){// ¸üÐÂ
		//if(parentNode->g+)
	   starnode* tmp = isInOpenVec(curnodeid);
       double newg = parentNode->g+newedge[g.matrix[parentNode->nodeid][tmp->nodeid]].edgeweightid;  //¸¸½ÚµãµÄG¼ÓÉÏ¸¸½Úµãµ½ÏÖÔÚopen±í½ÚµãµÄG
	   if(newg < tmp->g){
		   tmp->parent = parentNode;       //¸üÐÂ¸¸½Úµã£¬¸üÐÂf
		   parent[tmp->nodeid] = parentNode->nodeid;
		   tmp->g = newg;
		   tmp->h = distanceFromA2B(tmp,eNode);
		   tmp->f = tmp->g+tmp->h;
		   dist[tmp->nodeid] = tmp->f;  //ÏÖÔÚ½ÚµãµÄ¾àÀë
	   }
	    //printf("%d is in v_open g = %d\n",curnodeid, tmp->g);
	}else {   //²»ÔÚopen±íÀïÃæ£¬¶¨Òå¸¸½Úµã£¬g£¬h,f,²¢¼ÓÈëopen±íÖÐ
		// Ìí¼Óµ½¿ªÆôÁÐ±í
		starnode* tmpnode = new starnode(curnodeid,curnode.position_x,curnode.position_y);
		tmpnode->parent = parentNode;
		parent[tmpnode->nodeid] = parentNode->nodeid;
		tmpnode->g = parentNode->g+newedge[g.matrix[parentNode->nodeid][curnodeid]].edgeweightid;
		tmpnode->h = distanceFromA2B(tmpnode,eNode);
		tmpnode->f = tmpnode->g+tmpnode->h;
		dist[tmpnode->nodeid] = tmpnode->f;
		v_open.push_back(tmpnode);
		//printf("add %d v_open,g = %d\n",curnodeid,tmpnode->g);
	}
	return true;

}

list<starnode*> Guide::findPath(starnode* eNode){
	//printf("enter find Path\n");
	bool isFind = false;
    list<starnode*> resultvec;
	starnode* Node = NULL;
	while(v_open.size()>0){
		Node = findMinNode2EndInOpen(eNode) ;    //¿´¿´Ä¿µÄ½ÚµãÊÇ·ñÔÚopen±íÖÐ£¬Èç¹ûÔÚ
		//printf("Node:%d\n",Node->nodeid);
		int nodeid = Node->nodeid;
		int endpoint = eNode->nodeid;
		if(nodeid == endpoint){
			isFind = true;
			break;
		}

		for(int i = 0; i < g.node_num; i++ ){
			if(g.matrix[nodeid][i]>=0&&g.matrix[nodeid][i]<INFI_MAX) {    //Óë½ÚµãÖ®¼äÓÐÂ·´æÔÚ
				map<int,node>::iterator iter = nodeMap.find(i);
				if (check(iter->second, eNode, Node))    //²¢ÇÒÂú×ãÊÇÔÚÇ°·½µÄ½Úµã
				checkPath(iter->second,Node,eNode);      //½øÈëÑ°Â·µÄÈýÖÖ×´Ì¬£ºÔÚopen±í£¬ÔÚclose±í£¬»òÖØÐÂ¼ÓÈëÐÂ½Úµã
			}
		}
		//deletOpenVec(index);
		v_close.push_back(Node);
	}
	//cout << "isFind is " << isFind << endl;
	if(isFind){
			//getPath(resultvec,node);
		while(Node!= NULL){
			resultvec.push_front(Node);
			Node = Node->parent;
		}
	}
	//cout << "done!" << endl;
	return resultvec;
}

list<starnode*> Guide::searchpath(int startpoint, int endpoint){  // ËÑË÷Â·¾¶
	deleteVec();

	dist[startpoint] = 0;
	parent[startpoint] = -1;
	map<int,node>::iterator iter = nodeMap.find(startpoint);
	//printf("%d:%d,%d\n",startpoint, iter->second.position_x,iter->second.position_y);
	starnode* sNode = new starnode(startpoint,iter->second.position_x,iter->second.position_y);
	iter = nodeMap.find(endpoint);
	//printf("%d:%d,%d\n",endpoint, iter->second.position_x,iter->second.position_y);
	starnode* eNode = new starnode(endpoint,iter->second.position_x,iter->second.position_y);
	v_open.push_back(sNode);   //½«¿ªÊ¼½Úµã·ÅÈëopen±í
	return findPath(eNode);     //¿ªÊ¼Ñ°Â·
}


/***************ÔÚopen ÀïÕÒÒ»¸öµ½ÖÕµã×îÐ¡µÄ½áµã**********/


/*********³·ÏúÓÃnew¿ª±ÙµÄ¿Õ¼ä**********/


void Guide::loadMap()
{
	ifstream fin("data.txt");
	string line;
	getline(fin, line);
	stringstream linebuf;
	linebuf << line;
	linebuf >> g.node_num >> g.edge_num;
	cout << g.node_num << "," << g.edge_num << endl;   //Êä³ö½ÚµãÊýÄ¿ºÍ±ßÊýÄ¿

	for (int i = 0; i < g.edge_num; i++){     //Êä³ö±ßÐÅÏ¢
		getline(fin, line);
		stringstream buf;
		buf << line;

		buf >> newedge[i].edgeid >> newedge[i].frontnodeid >> newedge[i].lastnodeid >> newedge[i].edgeweightid;
		cout << newedge[i].edgeid << ":" << newedge[i].frontnodeid << "," << newedge[i].lastnodeid << "," << newedge[i].edgeweightid << endl;
		g.matrix[newedge[i].frontnodeid][newedge[i].lastnodeid] = newedge[i].edgeid;    //¶¨ÒåµÄ±ßµÄÐÅÏ¢
		edgeMap.insert(pair<int, edge>(newedge[i].edgeid, newedge[i]));
	}


	for (int i = 0; i<g.node_num; i++){   //Êä³ö½ÚµãÐÅÏ¢
		getline(fin, line);
		stringstream nodebuf;
		nodebuf << line;
		nodebuf >> newnode[i].m_index >> newnode[i].position_x >> newnode[i].position_y >> newnode[i].nodename;
		cout << newnode[i].nodename << " (" << newnode[i].position_x << "," << newnode[i].position_y <<  ")" << endl;
	    nodeMap.insert(pair<int, node>(newnode[i].m_index, newnode[i]));//ÏòÈÝÆ÷ÖÐ²åÈë½ÚµãÔªËØ
	}
	fin.close();
}

int Guide::getIndexStart(double point_x, double point_y) {   //
    double minv = -1;
    int ans = -1;
    for (int i=0; i<g.node_num; i++) {    //Ä£ºýÊäÈë
        double d = sqrt((newnode[i].position_x-point_x)*(newnode[i].position_x-point_x)+
                        (newnode[i].position_y-point_y)*(newnode[i].position_y-point_y));
        if (minv == -1 || d < minv) minv = d, ans = i;
    }
    return ans;
}

void *Guide::guide(void *arg)
{
	Guide *guide = (Guide*)arg;
	double sum;
	double point_x, point_y;
	list<starnode*> listNode;
	std::pair<int,int> sour,dest;
	int id;
	while (true){    
		if((guide->msgQueue)->startGuide(id,sour,dest) == -1)	//no guide request
			continue;
		// guide->startPoint_index = guide->getIndexStart(get<0>(sour),get<1>(sour));
		// guide->endPoint_index = guide->getIndexStart(get<0>(dest),get<1>(dest));
		guide->startPoint_index = guide->getIndexStart(sour.first,sour.second);
		guide->endPoint_index = guide->getIndexStart(dest.first,dest.second);
      
        // A* 查找算法
        guide->dist.clear();
        guide->parent.clear();
        listNode = guide->searchpath(guide->startPoint_index,guide->endPoint_index);

	   //输出路径
		std::vector<std::pair<int,int> > path;
        for (auto it=listNode.begin(); it!=listNode.end(); ++it) 
        {
        	path.push_back(std::make_pair((*it)->positionx,(*it)->positiony));
			cout<<(*it)->positionx<<" "<<(*it)->positiony<<"->"<<endl;	
        }
		(guide->msgQueue)->finishGuide(id,path);

		#if TEST
   		if(guide->test()==-1)
   			cout<<"error in run"<<endl;
		#endif
	}
}


int Guide::test(void)
{
	struct timeval tv;
	if(gettimeofday(&tv,(struct timezone*)NULL)==-1)
	{
		perror("in Guide::test, gettimeofday");
		return -1;
	}
	guideDuring.push_back(tv);
	receiveBufSize.push_back(msgQueue->getGuideNum());	
	return 0;
}

void Guide::record(void)
{
	ofstream out;
	out.open("timeOfGuide.txt");
	if(guideDuring.size() != receiveBufSize.size())
	{
		cout<<"size of guideDuring , receiveBufSize and sendBufsize are not same!"<<endl;
		return;
	}
	int dur;
	out<<"   dur         receiveBufSize"<<endl;
	for(size_t a = 0;a<guideDuring.size()-1;++a)
	{
		dur = (guideDuring[a+1].tv_sec - guideDuring[a].tv_sec)*1000000
			+ guideDuring[a+1].tv_usec - guideDuring[a].tv_usec;
		out<<dur<<"us          "<<receiveBufSize[a]<<"items"<<endl;
	}
	out.close();
}


// int main(void)
// {
// 	Guide guide;
// 	sleep(1000);
// }
