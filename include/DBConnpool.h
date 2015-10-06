#ifndef _CONNECTION_POOL_H
#define _CONNECTION_POOL_H
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <pthread.h>
#include <list>

using namespace std;
using namespace sql;

class ConnPool{
private:
    int curSize;
    int maxSize;
    string username;
    string password;
    string url;
    list<Connection*> connList;
    pthread_mutex_t lock;
    static ConnPool *connPool;
    Driver *driver;

    Connection* CreateConnection();//创建一条连接
    void InitConnection(int iInitialSize);//初始化数据库连接池
    void DestroyConnPool();//销毁数据库连接池
    void DestroyConnection(Connection *conn);
    ConnPool(string url, string user, string password, int maxSize);

public:
    ~ConnPool();
    Connection* GetConnection();//从连接池中获得一条连接
    void ReleaseConnection(Connection *conn);
    static ConnPool *GetInstance();
};
#endif
