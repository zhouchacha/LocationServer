#ifndef DTAT_BASE_H
#define DTAT_BASE_H

#include <vector>
#include <map>
#include <tuple>
#include <Guide.h>
#include <ConnPool.h>
using namespace std;

class Database
{
    public:
        /*
        *RegisterHandle主要是负责数据库用户的注册,
        *@param username,usertel与password参数,
        *@return 返回500,代表注册成功,返回501,代表username或usertel与已照注册,返回502,代表注册失败,请重试


        *NameLoginHandle方法表示使用用户名登陆,
        *@return 返回510,表示用户名登陆成功;返回511,代表密码错误;返回512,代表没有该用户名

        *TelLoginHandle表示使电话号码登陆,
        *@return 返回510,表示用户名登陆成功;返回511,代表密码错误;返回512,代表没有该用户名

        *TelNotLoginHandle表示临时用户暂时使用为注册的电话号码登陆,
        *@return 返回510,表示登陆成功;返回514,代表登陆失败,数据库插入失败,请重新尝试;返回513,该电话号码已注册,登陆失败

        *NameLogoutHandle表示使用用户名退出登陆,
        *@return 返回310,退出登陆成功;返回311,退出登陆失败

        *TelLogoutHandle表示使用电话号码退出登陆,
        *@return 返回310,退出登陆成功;返回311,退出登陆失败

        *LocationHandleData表示获取数据库中的指纹定位参考数据,
        *@param vector<Locate>&vect用来存储从数据库中读取的数据(每一条都包含FingerPrint表中的所有数据),
        *@return 返回1540,表示读取数据成功,返回1541,表示读取数据失败

        *GuideHandleData表示从数据库中读取导引所需数据,
        *@param nodeMap用户存储抽象节点信息,edgeMap用于存储节点间的边信息;
        *@return 返回1510,表示读取数据成功;返回1511,表示读取数据失败

        *UserPositionsHandle用于存储用户读走过的位置信息,
        *@return 当返回400,代表首次记录该用户的历史位置信息;返回401,表示用户为登陆或者以临时用户身份登陆,不用记录位置信息;返回402,表示用户记录历史位置信息成功;返回403表示用户移动距离太小,不用记录信息

        *GetHistory用于获取用户的某条历史记录,
        *@param data用来存储每条路径的日期,开始时间与结束时间,roadPosition用于存储用户的路径(该路径包含多个位置),
        *位置格式为[“x”:1, ”y”:2, “time”:”13:45”],
        *@return 返回1560,表示获取该条路径历史记录成功;返回1561,表示获取该条历史记录失败

        *deleteOneHistory用于用户删除某条用户的历史记录,
        *@param 其中data中存的是用户某条路径开始的日期,开始时刻,结束时刻,
        *@return 返回1575,表示删除成功,返回1576,表示删除失败

        * deleteAllHistory用于删除某用户的所有的历史记录,
        *@return 返回1570,表示删除所有记录成功,返回1571,删除失败
        */
        static int RegisterHandle(string &usertel, string &username, string &password);
        static int NameLoginHandle(string &username, string &password);
        static int TelLoginHandle(string &usertel, string &password);
        static int  TelNotLoginHandle(string &usertel);
        static int  NameLogoutHandle(string &username);
        static int  TelLogoutHandle(string &usertel);
        
        static int getLocationData(multimap<string, tuple<string,int,int> >&fingers);
        static int getGuideData(map<int,node > &nodeMap,map<int,edge> &edgeMap);
       
        static int recordUserPosition(string &username, int positionX, int positionY);
        static int GetHistory(string &username,int num,vector<string> &data,vector<vector<string> >&roadPosition);
        static int  deleteOneHistory(string &username,vector<string> &data);
        static int  deleteAllHistory(string &username);
        static ConnPool* pool;

};
#endif
