/*
数据库连接池创建过程函数调用顺序：
GetInstance() -> ConnPool() -> InitConnection() -> CreateConnection()

我们会调用GetConnection()来得到一条连接 ReleaseConnection()来释放掉该连接

*/
#include <define.h>
#include <stdexcept>
#include <exception>
#include <iostream>
#include <stdio.h>
#include <ConnPool.h>

using namespace std;
using namespace sql;

ConnPool *ConnPool::connPool = NULL;
const char *DBidentify="tcp://192.168.1.2:3306";
const char *DBuser="root";
const char *DBpasswd="root123";

ConnPool::ConnPool(string url, string userName,string password, int maxSize) {
    cout<<"DBConnpool::ConnPool()"<<endl;
    this->maxSize = maxSize;
    this->curSize = 0;
    this->username = userName;
    this->password = password;
    this->url = url;

    //线程锁初始化
    lock = PTHREAD_MUTEX_INITIALIZER;

    try{
        this->driver=get_driver_instance();
    }
    catch(sql::SQLException &e)
    {
        cout<<"驱动连接出错"<<endl;
    }
    catch(std::runtime_error &e)
    {
        perror("运行出错了\n");
    }
    //初始化时创建允许最大连接数的一半
    this->InitConnection(maxSize/2);
}

ConnPool *ConnPool::GetInstance() {
    cout<<"DBConnpool::GetInstance()"<<endl;
    if(connPool == NULL)
    {
        connPool = new ConnPool(DBidentify,DBuser,DBpasswd,20);
    }
    return connPool;
}

void ConnPool::InitConnection(int iInitialSize) {
    cout<<"DBConnpool::InitConnection()"<<endl;
    Connection *conn;
    pthread_mutex_lock(&lock);
    for(int i=0; i<iInitialSize; i++)
    {
        conn = this->CreateConnection();
        if(conn) {
            connList.push_back(conn);
            ++(this->curSize);
            cout<<"成功创建了第"<<this->curSize<<"条连接"<<endl;
        }
        else
        {
            perror("创建CONNECTION出错");
        }
    }
    pthread_mutex_unlock(&lock);
}

Connection* ConnPool::CreateConnection() {
    //cout<<"DBConnpool::CreateConnection()"<<endl;
    Connection *conn;
    try{
        conn = driver->connect(this->url,this->username,this->password);
        return conn;
    }
    catch(sql::SQLException&e)
    {
        perror("创建连接出错");
        return NULL;
    }
    catch(std::runtime_error &e)
    {
        perror("运行时出错");
        return NULL;
    }
}

Connection* ConnPool::GetConnection() {
    //cout<<"DBConnpool::GetConnection()"<<endl;
    Connection *con;
    pthread_mutex_lock(&lock);
    //如果连接池中还有连接，则拿出该连接，并将它从连接池中移除
   //cout<<"目前有"<<connList.size()<<"条连接可用."<<endl;
    if(connList.size()>0)
    {
        con=connList.front();
        connList.pop_front();
        if(con->isClosed())
        {
            delete con;
            con = this->CreateConnection();
        }
        if(con == NULL)
        {
            --curSize;
        }
        pthread_mutex_unlock(&lock);
        //cout<<"从数据库连接池中拿出一条连接，还剩余"<<connList.size()<<"条可用."<<endl;
        return con;
    }
    //如果连接池中当前并没有连接
    else{
        //如果当前连接数小于连接池最大连接数
        if(curSize < maxSize) {
            con = this->CreateConnection();
            if(con)
            {
                ++curSize;
                cout<<"数据库连接池已经没有可用连接，且当前已建立连接小于上限数，建立新连接."<<endl;
                pthread_mutex_unlock(&lock);
                return con;
            }
            else
            {
                pthread_mutex_unlock(&lock);
                return NULL;
            }
        }
        //如果当前连接数已经达到连接池最大连接数，则没办法再创建
        else{
            cout<<"数据库连接池已经没有可用连接，且当前已建立连接达到上线，不可再建新连接."<<endl;
            pthread_mutex_unlock(&lock);
            return NULL;
        }
    }
}

void ConnPool::ReleaseConnection(sql::Connection * conn) {
    cout<<"DBConnpool::ReleaseConnection()"<<endl;
    if(conn)
    {
        pthread_mutex_lock(&lock);
        connList.push_back(conn);
        cout<<"用户释放掉了一条数据库连接后可用连接数:"<<connList.size()<<endl;
        pthread_mutex_unlock(&lock);
    }
}

ConnPool::~ConnPool() {
    this->DestroyConnPool();
}

void ConnPool::DestroyConnPool() {
    cout<<"DBConnpool::DestroyConnPool()"<<endl;
    list<Connection*>::iterator icon;
    pthread_mutex_lock(&lock);
    for(icon=connList.begin();icon!=connList.end();++icon)
    {
        this->DestroyConnection(*icon);
    }
    curSize=0;
    connList.clear();
    pthread_mutex_unlock(&lock);
}

void ConnPool::DestroyConnection(Connection* conn) {
    cout<<"DBConnpool::DestroyConnection()"<<endl;
    if(conn)
    {
        try{
            conn->close();
        }
        catch(sql::SQLException &e)
        {
            perror(e.what());
        }
        catch(std::exception &e)
        {
            perror(e.what());
        }
        delete conn;
    }
}
