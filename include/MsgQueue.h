#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <pthread.h>
#include <json/json.h>
#include <vector>
#include <tuple>
#include <queue>
#include <utility>
#include <string>
#include <map>

class TcpUser;

using std::string;

class MsgQueue
{
public:
	MsgQueue(){};

	~MsgQueue(){};

	/*把导引请求加入到待定位队列,Json::Value格式：{”rssi”:”rssi”}
	检查该用户是否在MsgQueue的users当中。如果不在则加入。
	*/
	void addLocItem(int id,TcpUser* ptcpuser,const Json::Value &value);

	/*
	描述：开始定位，被定位线程调用来获取定位请求。但并不删除该定位请求。
	参数：id为请求标识，rssi为请求的rssi值, 二者都为引用参数
	返回值：成功返回 0， 没有获得请求则返回 -1
	*/
	int startLoc(int &id, string &rssi);

	/*
	描述：定位完成，被定位线程调。通过对比id与定位队列第一个请求的id来判断该用户是否已经断开，
	如果不相等则认为断开，立即返回。如果相等则认为没有断开，把定位结果加入到id对应的结果队列里
	并删除定位请求队列的第一个元素。
	参数：id为请求标识，loc 为定位结果,loc的格式为：tuple<x,y,z>。
	*/
	void finishLoc(int id, std::tuple<int,int,int>);

	/*
	把导引请求加入到待导引队列
	Json::Value格式： {”sour”:{“x”:1.1,”y”:1.1},”dest”:{“x”:1.1,”y”:1.1}}
	检查该用户是否在MsgQueue的users当中。如果不在则加入。
	*/
	void addGuideItem(int id,TcpUser* ptcpuser,const Json::Value &value);

	/*
	描述：开始导引，被导引线程调用来获取导引请求。但不删除该请求。
	参数：id为请求标识，tuple<int,int> &sour 为源坐标，sour的第一
		个元素是X坐标，第二个为Y坐标。tuple<int,int> &dest为目的坐
		标，三者都为引用参数
	返回值：成功返回 0， 没有获得请求则返回 -1
	*/
	int startGuide(int &id, std::pair<int,int> &sour, std::pair<int,int> &dest);

	/*
	描述：导引完成，通过对比id与导引队列第一个请求的id来判断该用户是否已经断开，
	如果不相等则认为断开，立即返回。如果相等则认为没有断开，把导引结果加入到id对应的结果队列里
	并删除导引请求队列的第一个元素。
	参数：id为请求标识，value 为导引结果。
	pair<int,int>的格式为：first为x,second为y
	*/
	void finishGuide(int id, const std::vector<std::pair<int,int> >& path);
	
	/**
	 * 描述：返回id好的对应的定位和导引结果，并转换成字符串	
	 * @param  strdata 引用参数，返回信息
	 * @return         成功返回ture，失败返回fal
	 */
	bool getMsg(int id,Json::Value &jMsg);

	/*
	描述：清除ID为id的用户的定位、导引请求和结果，
	参数:用户的ID。
	*/
	void clearRqst(int id);

	/**
	 * 获取当前未定位的请求个数
	 * @return  个数
	 */
	inline int getLocateNum(void)
	{
		return (int)LocateVec.size();
	};

	/**
	 * 获取当前未导引的请求个数
	 * @return  个数
	 */
	inline int getGuideNum(void)
	{
		return (int)GuideVec.size();
	};


private:
	//定位请求，Json::Value格式：{”rssi”:”rssi”}
	std::vector<std::pair<int,Json::Value> > LocateVec;

	//导引请求，Json::Value格式： {”sour”:{“x”:1.1,”y”:1.1},”dest”:{“x”:1.1,”y”:1.1}}
	std::vector<std::pair<int,Json::Value> > GuideVec;

	//定位结果，LocateVec[0]为ID为0的用户的结果队列
	//Json::Value格式：{“x”:12,”y”:29,”z”:1}
	std::vector<std::queue<Json::Value> > LocatedVec;

	//导引结果，LocateVec[0]为ID为0的用户的结果队列。
	//Json::Value格式： {z:2, path:[{“x”:20.1,”y”:40.1}, {“x”:26.1,”y”:42.1}…]}
	std::vector<std::queue<Json::Value> > GuidedVec;

	//用来存放用户对象指针与用户id对。
	std::map<int,TcpUser*> users;
};

#endif