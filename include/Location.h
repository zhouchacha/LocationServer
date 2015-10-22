#ifndef LOCATION_H
#define LOCATION_H

#include <MsgQueue.h>

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <map>
using std::multimap;

#include <tuple>
using std::tuple;

#include <utility>
using std::pair;
 
#include <Point.h>

typedef pair<string, double> PAIR; 

class Location
{
public:
	Location(MsgQueue *msgQueue):msgQueue(msgQueue){};
	~Location();


	/**
	 * @brief 初始化
	 * @details 调用getData获取指纹数据，创建定位线程。
	 */
	void init(void);


	/**
	 * @brief 定位线程函数
	 * @details 定位线程的回掉函数
	 * 
	 * @param arg 传入的参数，实际传入的参数为this指针，指向当前Location对象。
	 * @return 无
	 */
	static void* run(void *arg);


	/**
	 * @brief 定位
	 * @details 通过输入参数，通过定位算法实现定位。
	 * 
	 * @param rssiInfo 待定位点测的的rssi数组
	 * @param fingers 指纹数据
	 * 
	 * @return 定位结果，依次分别为x,y.
	 */
	pair<int,int> Locating(const string& rssiInfo);

	int static cmp(const PAIR &x, const PAIR &y); 

	/**
	 * @brief 获取当前计算需要的指纹库
	 * @details 根据mac1,mac2为最大的RSSI对应的指纹返回。
	 * 
	 * @param mac1 用户发来RSSI中第一大的值对应的mac
	 * @param mac2 用户发来RSSI中第二大的值对应的mac
	 * @param curFinger mac1,mac2为最大rssi对应的mac
	 */
	void getCurFinger(string mac1,string mac2,std::map<int,std::vector<string> > &curFinger,std::vector<Point> &points);


	/**
	 * 描述：获取当前时间并存放于locateDuring中，获取定位qq队列中当前请求
	 * 的个数并存于receiveBufSize中，获取该用户目前返回队列里数据的个数，并
	 * 存于sendBufsize中。并把当前定位的位置加入到loc_record中。
	 * 输入：x :x坐标， y:y坐标 
	 * 输出：0 --成功，-1 --失败
	 */
	int test(int x,int y);
	


	/**
	 * 记录测试到的数据，包括时间，剩余请求数。
	 */
	void record(void);


private:
	/**
	 * dbfingers中放的是指纹数据，格式：string为该条数据中rssi最大的mac地址。
	 * tuple中的string为指纹数据，第一个int为x,第二个int为y.
	 */
	multimap<string, tuple<string ,int ,int> > fingers;
	MsgQueue *msgQueue;

	/**
	 * 用来测试定位时间，locateDuring记录每次从请求队列里取出请求时的时间。
	 *receiveBufSize记录此时的请求队列里数据的个数。 
	 */
	vector<struct timeval> locateDuring;
	vector<int> receiveBufSize;

	/**
	 * 记录定位精度，第一个为X，第二个为y;
	 */
	std::vector<std::pair<int,int> > loc_record;
};

#endif