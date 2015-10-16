#ifndef LOCATION_H
#define LOCATION_H

#include <MsgQueue.h>

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <map>
using std::multimap;

#include <utility>

class Location
{
public:
	Location(MsgQueue *msgQueue)：msgQueue(msgQueue){};
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
	pair<int,int> Location(const string& rssiInfo,,const multimap<int,tuple<string, int, int>& fingers);


	/**
	 * 描述：获取当前时间并存放于locateDuring中，获取定位qq队列中当前请求
	 * 的个数并存于receiveBufSize中，获取该用户目前返回队列里数据的个数，并
	 * 存于sendBufsize中。
	 * 输入：无  输出：0 --成功，-1 --失败
	 */
	int test(void);
	


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
};

#endif