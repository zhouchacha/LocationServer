#include <json/json.h>
#include <vector>

#include <TcpUser.h>
#include <define.h>
#include <TcpServer.h>
#include <Database.h>

const int REGISTER = 1;//用户注册
const int NAMELOGIN = 10;//用户使用username登录
const int TELLOGIN = 11;//用户使用usertel登录
const int TELNOTLOGIN = 12;//临时用户使用usertel登录

const int NAMELOGOUT = 210;//用户名用户登出操作
const int TELLOGOUT = 211;//电话用户登出操作

const int GUIDE = 1010; //导航
const int LOCATION = 1040;//定位查询

const int FLIGHTNUM = 1020; //乘车口查询
const int TICKETINF = 1030; //票务信息查询

const int ROADRECORD = 1060;//路径历史记录查询
const int DELETERECORD = 1070;//删除路径历史纪录
const int DELETEONERECORD = 1075;//删除特定路径历史记录

const int SERVERQUIT = 9999;    //服务器退出


TcpUser::TcpUser(int fd,TcpServer *tcpServer)
                :sock_fd(fd),tcpServer(tcpServer),
                msgQueue(tcpServer->msgQueue),username(""),
                tel("")
{
    id = onLineNum;
    onLineNum++;
    debug("create TcpUser success");
}


int TcpUser::onLineNum = 0;

int TcpUser::Init() 
{
    if ((readEvt = event_new(tcpServer->base, sock_fd, EV_READ | EV_PERSIST, OnRead, this))== NULL) 
    {
        debug("create read event error");
        return -1;
    }
    event_add(readEvt, NULL);

    if ((writeEvt = event_new(tcpServer->base, sock_fd, EV_WRITE, OnWrite, this)) == NULL) 
    {
        debug("create write event error");
        return -1;
    }
    return 0;
}


void TcpUser::DataHandle(string &data)
{
    debug("handle data");
    // debug("handle data %s",data.c_str());
    Json::Reader reader;
    Json::Value value;
    int typecode;
    if(reader.parse(data, value))
        typecode = value["typecode"].asInt();
    
    switch(typecode) 
    {
        //关闭服务器
        case SERVERQUIT:
            debug("recive an quit request");
            tcpServer->QuitServer();
            break;

        //用户注册
        case REGISTER: {
            debug("DB receive register data");
            string usertel = value["usertel"].asString();//content中默认只有用户名，
            string username = value["username"].asString();
            string password = value["password"].asString();

            int ret = Database::RegisterHandle(username, username, password);   //传用户名到电话号码参数

            // int ret = Database::RegisterHandle(usertel, username, password);   //传用户名到电话号码参数
            Json::Value jRet;
            jRet["typecode"] = ret;
            sendMsg.push(jRet);
            PendingWriteEvt();
            break;
        }

        //使用username进行登录
        case NAMELOGIN:{
            debug("login with name");
            string username = value["username"].asString();
            string password = value["password"].asString();
            this->username = username;
            int ret = Database::NameLoginHandle(username, password);
            Json::Value jRet;
            jRet["typecode"] = ret;
            sendMsg.push(jRet);
            PendingWriteEvt();
            break;
        }

        //使用usertel进行登录
        case TELLOGIN:{
            debug("DB receive tellogin data");
            string usertel = value["usertel"].asString();
            string password = value["password"].asString();
            int ret = Database::TelLoginHandle(usertel, password);
            Json::Value jRet;
            jRet["typecode"] = ret;
            sendMsg.push(jRet);
            PendingWriteEvt();
            break;
        }

        //未注册用户使用usertel登录
        case TELNOTLOGIN: {
            debug("DB receive telnotlogin data");
            string usertel = value["usertel"].asString();
            int ret = Database::TelNotLoginHandle(usertel);
            Json::Value jRet;
            jRet["typecode"] = ret;
            sendMsg.push(jRet);
            PendingWriteEvt();
            break;
        }

        //用户名登出
        case NAMELOGOUT: {
            debug("DB receive userlogout data");
            string username = value["username"].asString();
            int ret = Database::NameLogoutHandle(username);
            Json::Value jRet;
            jRet["typecode"] = ret;
            sendMsg.push(jRet);
            PendingWriteEvt();
            break;
        }

        //用户电话登出
        case TELLOGOUT: {
            debug("DB receive userlogout data");
            string usertel = value["usertel"].asString();
            int ret = Database::TelLogoutHandle(usertel);
            Json::Value jRet;
            jRet["typecode"] = ret;
            sendMsg.push(jRet);
            PendingWriteEvt();
            break;
        }

        // 定位
        case LOCATION: {
            debug("DB receive location data");
            Json::Value rssi;
            rssi["rssi"] = value["rssi"];
            msgQueue.addLocItem(id,this,rssi);
            break;
        }

        // 导引
        case GUIDE:{
            debug("DB receive guide data");
            Json::Value sd;
            sd["sour"] = value["sour"];
            sd["dest"] = value["dest"];
            msgQueue.addGuideItem(id,this,sd);
            break;
        }

        // 读路径信息
        case ROADRECORD:{
            debug("DB recive road record");
            Json::Value object,array,item,path,pathitem;
            int nums = 0;
            for(int i=value["start_item"].asInt();i<=value["end_item"].asInt();i++)
            {
                std::vector<string> strVct;
                std::vector<vector<string> > roadPath;
                if(Database::GetHistory(username,i,strVct,roadPath) == 1561)
                {
                    debug("get history errror");
                    continue;
                }
                for(auto iter=roadPath.begin();iter!=roadPath.end();iter++)
                {
                    stringstream ss;
                    ss<<(*iter)[0];
                    int x;
                    ss>>x;
                    pathitem["x"] = x;
                    ss<<(*iter)[1];
                    int y;
                    ss>>y;
                    pathitem["y"] = y;
                    pathitem["time"] = (*iter)[2];
                    path.append(pathitem);
                }
                item["path"] = path;
                item["date"] = strVct[0];
                item["starttime"] = strVct[1];
                item["endtime"] = strVct[2];
                array.append(item);  
                nums++;              
            }
            object["typecode"] = 1560;
            object["history_items"] = array;
            object["nums"] = nums;
            Json::FastWriter writer;
            sendMsg.push(object);
            PendingWriteEvt();
            break;
        }

        // 删除路径历史记录
        case DELETEONERECORD:
        case DELETERECORD:{
            debug("DB recive delete record");
            Json::Value object;
            if(typecode == DELETERECORD)
            {
                if(Database::deleteAllHistory(username) == -1)
                {
                    object["typecode"] = 1571;
                }
                object["typecode"] = 1570;
            }
            else
            {
                std::vector<string> strVct;
                strVct.push_back(value["data"].asString());
                strVct.push_back(value["starttime"].asString());
                strVct.push_back(value["endTime"].asString());
                if(Database::deleteOneHistory(username,strVct) == -1)
                {
                    debug("delete one history error");
                    object["typecode"] = 1576;
                }
                object["typecode"] = 1575;
            }
            Json::FastWriter writer;
            sendMsg.push(object);
            PendingWriteEvt();
            break;
        }

        default:{
            // debug("illegal request: %s",data.c_str());
           break;
        }
    }
}


void TcpUser::SetUserName(string name)
{
    this->username = name;
}

void TcpUser::SetUserTel(string tel)
{
    this->tel = tel;
}

int TcpUser::GetId(void) const
{
    return id;
}

void TcpUser::PendingWriteEvt()
{
    int pending = event_pending(writeEvt,EV_WRITE,NULL);
    if(pending != EV_WRITE)
        event_add(writeEvt,NULL);
}

int TcpUser::MyRead(string &str) const
{
    short int len;
    if (recv(sock_fd, (char*) &len, 2, 0) >= 0) 
    {
        len = ntohs(len);
        if(len>0&&len<1024000)
        {
            NewDataArray newDataArray(len+1);
            char* data = newDataArray.GetArrayPtr();
            if(recv(sock_fd,data,len,0) == len)
            {
                str = data;
                return 0;
            }
        }
    }
    return -1;
}

void TcpUser::OnRead(int sock, short what, void *arg) 
{
    debug("receive data");
    string data;
    TcpUser* ptcpUser = (TcpUser*)arg;
    if( 0 == ptcpUser->MyRead(data) )
        ptcpUser->DataHandle(data);

    else
    {
        if(ptcpUser->username != "") //下线
            ptcpUser->DataHandle(ptcpUser->username);

        (ptcpUser->msgQueue).clearRqst(ptcpUser->id);
        (ptcpUser->tcpServer)->OffConnection(ptcpUser);
    }

}


int TcpUser::MyWrite(const string &str) const
{
    NewDataArray newDataArray(2+str.size()+1);
    char* data = newDataArray.GetArrayPtr();
    short int len = str.size();
    memcpy(data,&len,2);
    memcpy(data+2,str.c_str(),len);

    // if(send(sock_fd,data,len+2,0) != len+2)  //加长度首部
    if(send(sock_fd,str.c_str(),str.size(),0) != str.size())
    {
        debug("error occur in sending data");
    }
    debug("sended data:%s,",str.c_str());
}


void TcpUser::OnWrite(int sock, short what, void *arg) 
{   
    TcpUser *ptcpUser = (TcpUser*) arg;
    Json::Value jMsg;
    Json::FastWriter writer;
    string strdata;
    if((ptcpUser->sendMsg).size() != 0)
    {
        jMsg = (ptcpUser->sendMsg).front();
        strdata = writer.write(jMsg);
        (ptcpUser->sendMsg).pop();
        ptcpUser->MyWrite(strdata);
    }
    else
    {
        bool ret = (ptcpUser->msgQueue).getMsg(ptcpUser->id,jMsg);
        if(ret)
        {
            if(jMsg["typecode"].asInt() == 1540)
            int ret = Database::recordUserPosition(ptcpUser->username, jMsg["x"].asInt(), jMsg["y"].asInt());
            if(ret != 402)
            {
                debug("record user positin error");
            }
            strdata = writer.write(jMsg);
            ptcpUser->MyWrite(strdata);
        }    
    }
}

TcpUser::~TcpUser() 
{
    event_free(readEvt);
    event_free(writeEvt);
    close(sock_fd);
    if(username == "")
    {
       debug("a tcp user was deleted");
    }
    else
    {
        debug("user: %s was deleted",username.c_str());
    }
}
