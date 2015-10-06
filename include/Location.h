#ifndef LOCATION_H
#define LOCATION_H

#include <Point.h>
#include <MsgQueue.h>

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <map>
using std::map;

class Location
{
public:
	Location(MsgQueue *msgQueue);
	~Location();
	void getData(void);
	static void* run(void *arg);
	Point Location1(const string& rssiInfo, const vector<Point>& points,const map<int,vector<string>>& dbfinger);

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
	map<int, vector<string> > dbfingers;
	vector<Point> points;
	MsgQueue *msgQueue;

	/**
	 * 用来测试定位时间，locateDuring记录每次从请求队列里取出请求时的时间。
	 *receiveBufSize记录此时的请求队列里数据的个数。 
	 */
	vector<struct timeval> locateDuring;
	vector<int> receiveBufSize;
};

#endif