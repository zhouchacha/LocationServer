#include <Util.h>
#include <define.h>
#include <Location.h>
#include <Database.h>
#include <algorithm>
#include <sys/time.h>
#include <stdio.h>

#include <cstring>

#include <fstream>
using std::ofstream;

#include <json/json.h>
using Json::FastWriter;
using Json::Value;
using Json::Reader;


void Location::init(void)
{
	debug("get Location data");
	//获取指纹数据
	  Database::getLocationData(fingers);

	//for test
	  // debug("fingers size:%d",(int)fingers.size());
	for(auto temp = fingers.begin();temp != fingers.end();++temp)
	{
		auto range = fingers.equal_range(temp->first);
		// debug("mac:%s",(temp->first).c_str());
		for(auto item = range.first;item != range.second;++item)
		{
			// debug("string:%s, x:%d, y:%d",(get<0>(item->second)).c_str(),get<1>(item->second), get<2>(item->second));
		}
		temp = --(range.second);
	}


	pthread_t p;
	if(pthread_create(&p, NULL, run, this) == -1)
	{
		debug("create location error");
	}
	else
	{
		debug("create location success");
	}
}


Location::~Location()
{
#if TEST
	record(); //记录测试结果
#endif
}


void* Location::run(void *arg)
{
	debug("location thread is working");
	Location *thislo = (Location*)arg;
	int id = 0;
    string rssi = "";
    Value object;
    FastWriter writer;
   	while (true)
   	{
   		
   		//获取待定位数据
        if((thislo->msgQueue)->startLoc(id,rssi) == -1)
            continue;
      
        //定位
        pair<int,int> ret = thislo->Locating(rssi);

        //定位结果
        debug("after locate");
        (thislo->msgQueue)->finishLoc(id,make_tuple(ret.first,ret.second,2));
        
        //测试定位时间
		#if TEST
   		if(thislo->test(ret.first,ret.second)==-1)
   			cout<<"error in run"<<endl;
		#endif
    }
}

int Location::cmp(const PAIR &x, const PAIR &y)  
        {  
             return x.second > y.second;  
        } 

pair<int,int> Location::Locating(const string& rssiInfo){
	
	map<string, double> rssiMap = Util::StringToMap(rssiInfo);
	map<string, double>::const_iterator iter = rssiMap.begin();
    //排序  将map的key和value组成一个新的结构PAIR，一个PAIR型的vector存储map中的所有内容，对vector按照value值进行排序。  
    vector<PAIR> pair_vec;
    for (map<string, double>::iterator iter = rssiMap.begin(); iter != rssiMap.end(); ++iter)  
       {  
           pair_vec.push_back(make_pair(iter->first, iter->second));  
       }  
        sort(pair_vec.begin(), pair_vec.end(), cmp); 
	vector<PAIR>::iterator afteriter = pair_vec.begin(); 
    string mac1 = (*afteriter).first; 
	afteriter++;
    string mac2 = (*afteriter).first;    
    //  String mac3 = unworkapmac;  //关闭服务的AP的mac
	// debug("mac1:%s, mac2:%s",mac1.c_str(),mac2.c_str());

	//获取所需的指纹数据
	std::map<int,std::vector<string> > curFinger;
	std::vector<Point> points;
	getCurFinger(mac1,mac2,curFinger,points);
        int size = points.size();
        // debug("points size:%d",size);
 	int* weight = new int[size];
 	//数据库中所有ap   一个map常量
	string rssiap = "e4:d3:32:db:25:fe,-55;e4:d3:32:db:27:04,-57;e4:d3:32:db:26:3c,-59;e4:d3:32:db:26:fc,-59;e4:d3:32:db:26:f2,-58;b0:48:7a:5d:f3:28,-52;20:dc:e6:6b:73:46,-86;c0:61:18:7a:6f:5a,-43;c0:18:85:81:9a:e5,-76;30:49:3b:09:68:27,-57;30:49:3b:09:68:25,-67;20:dc:e6:6d:13:0e,-78;42:7c:8f:78:b8:e4,-73;30:49:3b:09:6b:49,-88;30:49:3b:09:6a:4f,-79;5c:ac:4c:be:47:de,-63;38:83:45:96:c7:6c,-90;d8:15:0d:38:5b:4e,-80;50:bd:5f:06:72:84,-91;e4:d3:32:85:87:30,-80;5c:63:bf:37:27:6c,-78;30:49:3b:09:6b:4b,-85;74:ea:3a:2f:6d:0a,-88;e4:d3:32:eb:f9:f2,-86;20:dc:e6:6b:36:10,-89;30:49:3b:09:6a:45,-86;42:7c:8f:78:b8:e4,-73;20:dc:e6:6d:13:0e,-78;00:36:76:14:a2:3d,-91;50:bd:5f:04:e1:ac,-91;e4:d3:32:e3:fc:38,-91;20:dc:e6:88:5b:5e,-91;38:83:45:47:51:84,-88;30:49:3b:09:6a:4b,-89;14:75:90:58:fa:ec,-92;30:49:3b:09:6a:61,-93;78:d7:52:cd:92:70,-91;e0:05:c5:b3:d3:9e,-91;30:49:3b:09:6a:59,-91;50:bd:5f:49:3f:02,-91;00:36:76:04:ef:ae,-91;20:dc:e6:54:ff:ae,-92;24:05:0f:3f:ea:07,-90;c4:17:fe:83:dc:d4,-91;ec:88:8f:4d:95:6c,-92;50:bd:5f:84:18:99,-91;78:a1:06:fd:41:84,-90;20:dc:e6:3b:dd:d6,-93;08:57:00:65:5e:7c,-84;80:89:17:e5:8e:e0,-96;74:ea:3a:26:10:86,-91;c8:e7:d8:45:6a:54,-88;ec:6c:9f:04:c6:64,-95;20:dc:e6:6a:37:ac,-92;30:49:3b:09:6a:69,-95;30:49:3b:09:6a:4d,-94;30:49:3b:09:6a:d1,-85;14:e6:e4:34:ab:c8,-95;00:36:76:01:6e:a8,-89;ec:88:8f:63:63:88,-91;c8:3a:35:03:3d:c0,-91;42:7c:8f:78:f2:b4,-87;00:25:86:51:b9:58,-86;30:49:3b:09:6a:6d,-95;2a:5d:60:e6:97:f4,-89;20:dc:e6:6b:35:a2,-93;20:dc:e6:69:6a:74,-90;50:bd:5f:04:ee:12,-91;08:10:74:67:5a:36,-93;40:16:9f:23:b9:ea,-91;30:49:3b:09:6b:81,-88;30:49:3b:09:68:19,-90;2a:59:f9:04:99:26,-93;6c:e8:73:b2:29:c6,-94;30:49:3b:09:68:1f,-97;30:49:3b:09:6a:49,-90;30:49:3b:09:6a:57,-90;ec:6c:9f:01:f3:d4,-90;30:49:3b:09:67:d9,-91;74:ea:3a:2f:6d:0a,-88;e4:d3:32:85:87:30,-80;00:23:cd:83:7d:a0,-93;00:0b:85:91:a3:4c,-93;78:52:62:1d:05:44,-91"  ;

 	map<string, double> mapap = Util::StringToMap(rssiap);
 	int apnum = 0;
    //比对ap的mac全库
 	while (iter!=rssiMap.end()){
 		string key = iter->first;  
 		map<string, double>::const_iterator iterap = mapap.find(key);
 		if (iterap != mapap.end()){
 			apnum++;
 			}
 			iter++;
 			}	
 		// cout<<"apnum:"<<apnum<<endl;
 	for (int i = 0; i < size; i++){
 		weight[i] = 0;
 	}
 	for (int i = 0; i < size; i++)
 	{
 		map<int,vector<string>>::const_iterator iter = curFinger.find(points[i].nodeid);
 		if (iter == curFinger.end())
 		{
		
 		}
 		vector<string> fingerOneDbRssi = iter->second;
 		for(string rssi:fingerOneDbRssi)
 		{
 			map<string, double>rssiMap2 = Util::StringToMap(rssi);
 			double result = Util::MapMatch(rssiMap, rssiMap2,apnum);
 			if (result>0.7)
 			{
 			double  weit =  Util::MapMatchW(rssiMap, rssiMap2);
 			int w = (int)weit;
 		    weight[i] += w ;
 			}
 		}
 	}
 	int sum = 0;
 	for (int k= 0; k < size; k++) 
 	{
 			sum += weight[k];
 	}
	
	Point point;	
 	for (int i = 0; i < size; i++) 
 	{
 		//cout<<"weight:"<<weight[i]<<endl;
 		point.xposition += points[i].xposition * weight[i]  / sum+0.001	;
 		point.yposition += points[i].yposition * weight[i]  / sum+0.001;
 	}

	//cout<<"location:"<<point.xposition<<", "<<point.yposition<<endl;
/*	
//knn算法
	int MaxPointNum[3]={0}; 
     
    void sort2(int b[],int NUM) 
   { 
    int i=0; 
    int j=0; 
    int MaxTemp; 
    int Record=0; 
    while(j < 3) 
    { 
        MaxTemp=b[0]; 
        Record=0; 
        for(i=0; i<NUM; i++) 
        { 
            if(b[i]>MaxTemp) 
            { 
                Record=i; 
                MaxTemp=b[i]; 
            } 
        } 
        b[Record]=0; 
        MaxPointNum[j]=Record; 
        j++; 
    } 
} 
    sort2(weight,size);
	for(int i=0; i<3; i++) 
    { 
        //cout<<MaxPointNum[i]<<endl; 
		//cout<<"weight:"<<weight[i]<<endl;
		point.xposition += points[MaxPointNum[i]].xposition * weight[MaxPointNum[i]]  / weight[MaxPointNum[0]]+weight[MaxPointNum[1]]+weight[MaxPointNum[2]]+0.001	;
		point.yposition += points[MaxPointNum[i]].yposition * weight[MaxPointNum[i]]  / weight[MaxPointNum[0]]+weight[MaxPointNum[1]]+weight[MaxPointNum[2]]+0.001;
	}

	//cout<<"location:"<<point.xposition<<", "<<point.yposition<<endl;
	return point;
}
*/

	return make_pair(point.xposition,point.yposition);
}

void Location::getCurFinger(string mac1,string mac2,std::map<int,std::vector<string> > &curFinger,std::vector<Point> &points)
{
	for(auto temp = fingers.begin();temp != fingers.end();++temp)
	{
		auto range = fingers.equal_range(temp->first);
		string st = (temp->first).substr(0,17);

		if((mac1.compare(st) == 0)||(mac2.compare(st) == 0))
		{
			for(auto rangeitem = range.first; rangeitem != range.second; ++rangeitem)
			{
				std::tuple<string,int,int> onefinger = rangeitem->second;
				auto item = points.begin();
				for(item; item != points.end();++item)
				{
					//如果坐标已经在points里，则把string加入到curfinder对应index的vector里
					if((item->xposition == get<1>(onefinger)) && item->yposition==get<2>(onefinger))
					{
						int index = item->nodeid;
						curFinger[index].push_back(get<0>(onefinger));
						break;
					}
				}
				//如果不在points里，则往points中加入新的point，则把string加入到curfinder对应index的vector里
				if(item == points.end())
				{
					int index = points.size();
					Point pt(index,get<1>(onefinger),get<2>(onefinger));
					points.push_back(pt);
					std::vector<string> fing;
					fing.push_back(get<0>(onefinger));
					curFinger.insert(std::make_pair(index,fing));
				}
			}
		}

		temp = --(range.second);
	}


	//for test
	// for(auto temp = curFinger.begin();temp != curFinger.end();++temp)
	// {
	// 	auto item = *temp;
	// 	auto flist = item.second;
	// 	for(auto i=flist.begin();i!= flist.end();++i)
	// 	{
	// 		debug("string:%s",(*i).c_str());
	// 	}
	// }
}

int Location::test(int x,int y)
{
	struct timeval tv;
	if(gettimeofday(&tv,(struct timezone*)NULL)==-1)
	{
		perror("in Location::test, gettimeofday");
		return -1;
	}
	locateDuring.push_back(tv);
	receiveBufSize.push_back(msgQueue->getLocateNum());	

	loc_record.push_back(std::make_pair(x,y));
	return 0;
}

void Location::record(void)
{
	ofstream out;
	out.open("timeOfLocation.txt");
	if(locateDuring.size() != receiveBufSize.size())
	{
		cout<<"size of locateDuring , receiveBufSize and sendBufsize are not same!"<<endl;
		return;
	}
	int dur;
	out<<"   dur         receiveBufSize"<<endl;
	for(size_t a = 0;a<locateDuring.size()-1;++a)
	{
		dur = (locateDuring[a+1].tv_sec - locateDuring[a].tv_sec)*1000000
			+ locateDuring[a+1].tv_usec - locateDuring[a].tv_usec;
		out<<dur<<"us          "<<receiveBufSize[a]<<"items"<<endl;
	}
	out.close();

	out.open("jindu.txt");
	for(int i=0;i<(int)loc_record.size();++i)
	{
		out<<i+1<<"	"<<loc_record[i].first<<"	"<<loc_record[i].second<<endl;
	}
}
