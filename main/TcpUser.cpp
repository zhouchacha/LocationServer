
// #include <utility>
// #include <iostream>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <memory.h>
// #include <time.h>	//for test time spent in each part
// #include <fstream>
#include <json/json.h>
#include <vector>

#include <TcpUser.h>
#include <define.h>
#include <TcpServer.h>

const int REGISTERTEL = 0;//
const int REGISTERNAME = 1;//用户注册
const int NAMELOGIN = 200;//用户使用username登录
const int TELLOGIN = 201;//用户使用usertel登录
const int TELNOTLOGIN = 202;//临时用户使用usertel登录
const int NAMELOGOUT = 210;//用户名用户登出操作
const int TELLOGOUT = 211;//电话用户登出操作
const int GUIDE = 1010; //导航
const int FLIGHTNUM = 1020; //乘车口查询
const int TICKETINF = 1030; //票务信息查询
const int LOCATION = 1040;//定位查询
const int LOCATION2 = 1235;//定位查询2
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
    debug("handle data %s",data.c_str());
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
        case REGISTERTEL:
        case REGISTERNAME: {
            debug("DB receive register data");
            string usertel = value["usertel"].asString();//content中默认只有用户名，
            string username = value["username"].asString();
            string password = value["password"].asString();
            // RegisterHandle(username, username, password);   //传用户名到电话号码参数
            break;
        }

        //使用username进行登录
        case NAMELOGIN:{
            debug("login with name");
            string username = value["username"].asString();
            string password = value["password"].asString();
            this->username = username;
            // NameLoginHandle(username, password);
            break;
        }

        //使用usertel进行登录
        case TELLOGIN:{
            debug("DB receive tellogin data");
            string usertel = value["usertel"].asString();
            string password = value["password"].asString();
            // TelLoginHandle(usertel, password);
            break;
        }

        //未注册用户使用usertel登录
        case TELNOTLOGIN: {
            debug("DB receive telnotlogin data");
            string usertel = value["usertel"].asString();
            // TelNotLoginHandle(usertel);
            break;
        }

        //用户名登出
        case NAMELOGOUT: {
            debug("DB receive userlogout data");
            string username = value["username"].asString();
            // NameLogoutHandle(username);
            break;
        }

        //用户电话登出
        case TELLOGOUT: {
            debug("DB receive userlogout data");
            string usertel = value["usertel"].asString();
            // TelLogoutHandle(usertel);
            break;
        }

        // 定位
        case LOCATION: {
            debug("DB receive location data");
            Json::Value rssi;
            rssi["rssi"] = value["rssi"];
            msgQueue.addLocItem(id,rssi);
            break;
        }

        // 导引
        case GUIDE:{
	        debug("DB receive guide data");
	        Json::Value sd;
	        sd["sour"] = value["sour"];
	        sd["dest"] = value["dest"];
	        msgQueue.addGuideItem(id,sd);
        	break;
        }

        // // 读路径信息
        // case ROADRECORD:{
        //     debug("DB recive road record");
        //     Value object,array,item,path,pathitem;
        //     int nums = 0;
        //     for(int i=value["start_item"].asInt();i<=value["end_item"].asInt();i++)
        //     {
        //         vector<string> strVct;
        //         vector<vector<string> > roadPath;
        //         if(GetHistory(ptcpUser->username,i,strVct,roadPath) == -1)
        //         {
        //             debug("get history errror");
        //             continue;
        //         }
        //         for(auto iter=roadPath.begin();iter!=roadPath.end();iter++)
        //         {
        //             stringstream ss;
        //             ss<<(*iter)[0];
        //             int x;
        //             ss>>x;
        //             pathitem["x"] = x;
        //             ss<<(*iter)[1];
        //             int y;
        //             ss>>y;
        //             pathitem["y"] = y;
        //             pathitem["time"] = (*iter)[2];
        //             path.append(pathitem);
        //         }
        //         item["path"] = path;
        //         item["date"] = strVct[0];
        //         item["starttime"] = strVct[1];
        //         item["endtime"] = strVct[2];
        //         array.append(item);  
        //         nums++;              
        //     }
        //     object["typecode"] = 1560;
        //     object["history_items"] = array;
        //     object["nums"] = nums;
        //     FastWriter writer;
        //     string data = writer.write(object);
        //     // ptcpUser->SendMsg(data);
        //     break;
        // }

        // // 删除路径历史记录
        // case DELETEONERECORD:
        // case DELETERECORD:{
        //     debug("DB recive delete record");
        //     Value object;
        //     if(typecode == DELETERECORD)
        //     {
        //         if(deleteAllHistory(ptcpUser->username) == -1)
        //         {
        //             object["typecode"] = 1571;
        //         }
        //         object["typecode"] = 1570;
        //     }
        //     else
        //     {
        //         vector<string> strVct;
        //         strVct.push_back(value["data"].asString());
        //         strVct.push_back(value["starttime"].asString());
        //         strVct.push_back(value["endTime"].asString());
        //         if(deleteOneHistory(ptcpUser->username,strVct) == -1)
        //         {
        //             debug("delete one history error");
        //             object["typecode"] = 1576;
        //         }
        //         object["typecode"] = 1575;
        //     }
        //     FastWriter writer;
        //     string data = writer.write(object);
        //     ptcpUser->SendMsg(data);
        //     break;
        // }

        default:{
            debug("illegal request: %s",data.c_str());
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

	// if(send(sock_fd,data,len+2,0) != len+2)	//加长度首部
	if(send(sock_fd,str.c_str(),str.size(),0) != str.size())
	{
		// debug("length:%d, send length:%d",str.length(),)
		debug("error occur in sending data");
	}
	debug("sended data:%s,",str.c_str());
}


void TcpUser::OnWrite(int sock, short what, void *arg) 
{	
	TcpUser *ptcpUser = (TcpUser*) arg;
	string strdata;
	// int size = ptcpUser->GetMsg(strdata);
	int size=1;
	
	ptcpUser->MyWrite(strdata);

	//只要sendMsgVec不为空，则循环触发writeEvt事件进行OnWrite发送
	if (size)
		event_add(ptcpUser->writeEvt, NULL);
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
