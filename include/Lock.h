#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>
#include <vector>
#include <initializer_list>
/**
 * 该类用来进行互斥的自动释放，避免形成死锁。
 * 在构造函数中对传入的互斥量进行加锁，析构函数中进行解锁，保证了互斥量一定能被释放
 * 注意：未考虑加锁失败和解锁失败的情况
 */
class Lock
{
public:
	Lock(std::initializer_list<pthread_mutex_lock> li)
	{ 
		for(auto ptr = li.begin();ptr != li.end();ptr++)
		{
			pthread_mutex_lock(lptr);
			lock.push_back(*ptr);
		}
	}

	void freeLock()
	{
		for(auto ptr = lock.begin(); ptr != lock.end(); ptr++)
		{
			pthread_mutex_unlock(*ptr);
		} 
	}

	~Lock()	{ freeLock(); }
private:
	std::vector<pthread_mutex_t> lock; 
};

#endif