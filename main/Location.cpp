#include <Util.h>
#include <Locate.h>
#include <define.h>
#include <Location.h>
#include <DBAnaysisData.h>

#include <sys/time.h>
#include <stdio.h>

#include <fstream>
using std::ofstream;

#include <json/json.h>
using Json::FastWriter;
using Json::Value;
using Json::Reader;

Location::Location(MsgQueue *msgQueue):msgQueue(msgQueue)
{
	// getData();
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
	record();
#endif
}

void Location::getData(void)	//把数据库里的指纹数据加载到dbfingers 和 points里
{
	int apid=1;
    for(apid=1; apid<8 ; apid++) {
        vector<Locate> v;
        vector<string> vs;
        // v = DBAnaysisData::Location2Handle(apid);
        Locate l = v[0];
        Point tmp(apid, l.getPositionx(), l.getPositiony());
        points.push_back(tmp);
        for(vector<Locate>::iterator it=v.begin(); it!=v.end(); it++)
        {
            Locate l = *it;
            vs.push_back(l.getFingerdetail());
        }
        dbfingers.insert(make_pair(apid,vs));
    }
}

void* Location::run(void *arg)
{
	Location *thislo = (Location*)arg;
	int id = 0;
    string rssi = "";
    Value object;
    FastWriter writer;
   	while (true)
   	{
        if((thislo->msgQueue)->startLoc(id,rssi) == -1)
            continue;

        // Point ret = thislo->Location1(rssi, thislo->points, thislo->dbfingers);
        
        // object["typecode"] = 1540;
        // // object["x"] = ret.xposition;
        // // object["y"] = ret.yposition;
        // object["z"] = 2;
        // // object["timestamp"]=value["timestamp"];
        // string strdata = writer.write(object);
        (thislo->msgQueue)->finishLoc(id,make_tuple(2,2,2));
        
#if 	TEST
        // cout<<"in locatefun,after locate,send strdata: "<<strdata<<endl;
   		if(thislo->test()==-1)
   			cout<<"error in run"<<endl;
#endif
    }
}

Point Location::Location1(const string& rssiInfo,  const vector<Point>& points, const map<int, vector<string>>& dbfinger){
	Point point;
	map<string, double> rssiMap = Util::StringToMap(rssiInfo);
	map<string, double>::const_iterator iter = rssiMap.begin();
	// cout<<"rssiMapSize:"<<endl;
// ofstream of("resoult.txt",ofstream::out|ofstream::app);
// static int num;
	int size = points.size();
	int* weight = new int[size];
	//数据库中所有ap   一个map常量
	string rssiap = "b0:48:7a:5d:f3:28,-52;20:dc:e6:6b:73:46,-86;c0:61:18:7a:6f:5a,-43;c0:18:85:81:9a:e5,-76;30:49:3b:09:68:27,-57;30:49:3b:09:68:25,-67;20:dc:e6:6d:13:0e,-78;42:7c:8f:78:b8:e4,-73;30:49:3b:09:6b:49,-88;30:49:3b:09:6a:4f,-79;5c:ac:4c:be:47:de,-63;38:83:45:96:c7:6c,-90;d8:15:0d:38:5b:4e,-80;50:bd:5f:06:72:84,-91;e4:d3:32:85:87:30,-80;5c:63:bf:37:27:6c,-78;30:49:3b:09:6b:4b,-85;74:ea:3a:2f:6d:0a,-88;e4:d3:32:eb:f9:f2,-86;20:dc:e6:6b:36:10,-89;30:49:3b:09:6a:45,-86;42:7c:8f:78:b8:e4,-73;20:dc:e6:6d:13:0e,-78;00:36:76:14:a2:3d,-91;50:bd:5f:04:e1:ac,-91;e4:d3:32:e3:fc:38,-91;20:dc:e6:88:5b:5e,-91;38:83:45:47:51:84,-88;30:49:3b:09:6a:4b,-89;14:75:90:58:fa:ec,-92;30:49:3b:09:6a:61,-93;78:d7:52:cd:92:70,-91;e0:05:c5:b3:d3:9e,-91;30:49:3b:09:6a:59,-91;50:bd:5f:49:3f:02,-91;00:36:76:04:ef:ae,-91;20:dc:e6:54:ff:ae,-92;24:05:0f:3f:ea:07,-90;c4:17:fe:83:dc:d4,-91;ec:88:8f:4d:95:6c,-92;50:bd:5f:84:18:99,-91;78:a1:06:fd:41:84,-90;20:dc:e6:3b:dd:d6,-93;08:57:00:65:5e:7c,-84;80:89:17:e5:8e:e0,-96;74:ea:3a:26:10:86,-91;c8:e7:d8:45:6a:54,-88;ec:6c:9f:04:c6:64,-95;20:dc:e6:6a:37:ac,-92;30:49:3b:09:6a:69,-95;30:49:3b:09:6a:4d,-94;30:49:3b:09:6a:d1,-85;14:e6:e4:34:ab:c8,-95;00:36:76:01:6e:a8,-89;ec:88:8f:63:63:88,-91;c8:3a:35:03:3d:c0,-91;42:7c:8f:78:f2:b4,-87;00:25:86:51:b9:58,-86;30:49:3b:09:6a:6d,-95;2a:5d:60:e6:97:f4,-89;20:dc:e6:6b:35:a2,-93;20:dc:e6:69:6a:74,-90;50:bd:5f:04:ee:12,-91;08:10:74:67:5a:36,-93;40:16:9f:23:b9:ea,-91;30:49:3b:09:6b:81,-88;30:49:3b:09:68:19,-90;2a:59:f9:04:99:26,-93;6c:e8:73:b2:29:c6,-94;30:49:3b:09:68:1f,-97;30:49:3b:09:6a:49,-90;30:49:3b:09:6a:57,-90;ec:6c:9f:01:f3:d4,-90;30:49:3b:09:67:d9,-91;74:ea:3a:2f:6d:0a,-88;e4:d3:32:85:87:30,-80;00:23:cd:83:7d:a0,-93;00:0b:85:91:a3:4c,-93;78:52:62:1d:05:44,-91"  ;
			
	//rssiMap , mpaap算出新加入apnum
	map<string, double> mapap = Util::StringToMap(rssiap);
	int apnum = 0;
//比较map1 中ap iter.first 是否在mapap中，iter++,sum++,为新加入的ap
	while (iter!=rssiMap.end()){
		string key = iter->first;  //取AP的MAC值
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
		map<int,vector<string>>::const_iterator iter = dbfinger.find(points[i].nodeid);
		if (iter == dbfinger.end())
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
	
	for (int i = 0; i < size; i++) 
	{
		//cout<<"weight:"<<weight[i]<<endl;
		point.xposition += points[i].xposition * weight[i]  / sum+0.001	;
		point.yposition += points[i].yposition * weight[i]  / sum+0.001;
	}

	//cout<<"location:"<<point.xposition<<", "<<point.yposition<<endl;
	return point;
}

int Location::test(void)
{
	struct timeval tv;
	if(gettimeofday(&tv,(struct timezone*)NULL)==-1)
	{
		perror("in Location::test, gettimeofday");
		return -1;
	}
	locateDuring.push_back(tv);
	receiveBufSize.push_back(msgQueue->getLocateNum());	
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
}
