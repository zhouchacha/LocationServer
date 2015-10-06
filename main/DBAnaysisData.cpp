#include <define.h>
#include <DBAnaysisData.h>
#include <MsgQueue.h>
#include <DBConnpool.h>

#include <sstream>
using std::stringstream;

#include <json/json.h>
using Json::Value;
using Json::Reader;
using Json::FastWriter;

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





extern ConnPool *pool;
extern MsgQueue msgQueue;
char database[8] = {"project"};

void DBAnaysisData::Handle(const string &content) 
{
    debug("handle data");
    Reader reader;
    Value value;
    int typecode;
    if(reader.parse(content, value)) {
        typecode = value["typecode"].asInt();
    }
    switch(typecode) {
        case SERVERQUIT:
        debug("recive an quit request");
            // exit;
            // ptcpUser->id = -1;
            // ptcpUser->SetLocItem(value);
            // msgQueue.addLocItem(ptcpUser);
            break;

        //用户注册
        case REGISTERTEL:
        case REGISTERNAME: {
            debug("DB receive register data");
            string usertel = value["usertel"].asString();//content中默认只有用户名，
            string username = value["username"].asString();
            string password = value["password"].asString();
            RegisterHandle(username, username, password);   //传用户名到电话号码参数
            break;
        }

        //使用username进行登录
        case NAMELOGIN:{
            debug("DB receive namelogin data");
            string username = value["username"].asString();
            string password = value["password"].asString();
            NameLoginHandle(username, password);
            break;
        }

        //使用usertel进行登录
        case TELLOGIN:{
            debug("DB receive tellogin data");
            string usertel = value["usertel"].asString();
            string password = value["password"].asString();
            TelLoginHandle(usertel, password);
            break;
        }

        //未注册用户使用usertel登录
        case TELNOTLOGIN: {
            debug("DB receive telnotlogin data");
            string usertel = value["usertel"].asString();
            TelNotLoginHandle(usertel);
            break;
        }

        //用户名登出
        case NAMELOGOUT: {
            debug("DB receive userlogout data");
            string username = value["username"].asString();
            NameLogoutHandle(username);
            break;
        }

        //用户电话登出
        case TELLOGOUT: {
            debug("DB receive userlogout data");
            string usertel = value["usertel"].asString();
            TelLogoutHandle(usertel);
            break;
        }

        // 定位
        case LOCATION: {
            debug("DB receive location data");
            ptcpUser->SetLocItem(value);
            msgQueue.addLocItem(ptcpUser);
            break;
        }

        // 导引
        case GUIDE:{
        debug("DB receive guide data");
        ptcpUser->SetGuideItem(value);
        msgQueue.addGuideItem(ptcpUser);
        break;
        }

        // 读路径信息
        case ROADRECORD:{
            debug("DB recive road record");
            Value object,array,item,path,pathitem;
            int nums = 0;
            for(int i=value["start_item"].asInt();i<=value["end_item"].asInt();i++)
            {
                vector<string> strVct;
                vector<vector<string> > roadPath;
                if(GetHistory(ptcpUser->username,i,strVct,roadPath) == -1)
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
            FastWriter writer;
            string data = writer.write(object);
            ptcpUser->SendMsg(data);
            break;
        }

        // 删除路径历史记录
        case DELETEONERECORD:
        case DELETERECORD:{
            debug("DB recive delete record");
            Value object;
            if(typecode == DELETERECORD)
            {
                if(deleteAllHistory(ptcpUser->username) == -1)
                {
                    object["typecode"] = 1571;
                }
                object["typecode"] = 1570;
            }
            else
            {
                vector<string> strVct;
                strVct.push_back(value["data"].asString());
                strVct.push_back(value["starttime"].asString());
                strVct.push_back(value["endTime"].asString());
                if(deleteOneHistory(ptcpUser->username,strVct) == -1)
                {
                    debug("delete one history error");
                    object["typecode"] = 1576;
                }
                object["typecode"] = 1575;
            }
            FastWriter writer;
            string data = writer.write(object);
            ptcpUser->SendMsg(data);
            break;
        }

        default:{
            debug("illegal request: %s",content.c_str());
           break;
        }
    }
}

//使用用户名或电话注册操作
void DBAnaysisData::RegisterHandle(string &usertel, string &username, string &password) {
    try {
        bool flag1 = false;
        bool flag2 = false;
        Value root;
        FastWriter fastwriter;
        string content;
        cout<<"ResisterHandle()"<<endl;
        Connection *conn = pool->GetConnection();
        //conn->setAutoCommit(false);
        conn->setSchema(database);
        PreparedStatement *pstmt;

        string selectsql1 = "SELECT * FROM users WHERE usertel=?";
        pstmt = conn->prepareStatement(selectsql1);
        pstmt->setString(1, usertel);
        ResultSet *rs= pstmt->executeQuery();
        //执行后发现users表中已存在usertel
        if(rs->next()){
            flag1 = true;
        }
        string selectsql2 = "SELECT * FROM users WHERE username=?";
        pstmt = conn->prepareStatement(selectsql2);
        pstmt->setString(1, username);
        rs= pstmt->executeQuery();
        if(rs->next()){
            flag2 = true;
        }
        cout<<"flag1="<<flag1<<endl;
        cout<<"flag2="<<flag2<<endl;
        if(flag1==true || flag2==true)
        {
            cout<<"register failed1."<<endl;
            root["typecode"] = Value(112);
            content = fastwriter.write(root);
        }
        else
        {
            string sql = "INSERT INTO users(usertel, username, password) values(?, ?, ?)";
            pstmt = conn->prepareStatement(sql);
            pstmt->setString(1, usertel);
            pstmt->setString(2, username);
            pstmt->setString(3, password);
            //cout<<"sql:"<<sql<<endl;
            if(pstmt->executeUpdate() > 0) {
                //conn->commit();
                cout<<"register success."<<endl;
                root["typecode"] = Value(111);
               // root["username"] = Value(username);
                content = fastwriter.write(root);
            }
            else {
                cout<<"register failed2."<<endl;
                root["typecode"] = Value(113);
                content = fastwriter.write(root);
            }
        }
        delete pstmt;
        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);
        cout<<content<<endl;
        ptcpUser->SendMsg(content);

    }
    catch(SQLException &e) {
        string s_error(e.what());
        cout<<e.what()<<endl;
    }
}

//username登录操作
void DBAnaysisData::NameLoginHandle(string &username, string &password) {
    try {
        Value root;
        FastWriter fastwriter;
        string content;
        PreparedStatement *pstmt;
        Connection *conn = pool->GetConnection();
        //conn->setAutoCommit(false);
        conn->setSchema(database);

        string sql = "SELECT * FROM users WHERE username=?";
        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, username);
        ResultSet *rs= pstmt->executeQuery();
        if(rs->next()){
            cout<<rs->getString(1)<<endl;
            cout<<rs->getString(2)<<endl;
            cout<<rs->getString(3)<<endl;
            cout<<rs->getString(4)<<endl;
            int loginNum=rs->getInt(9);
            loginNum++;
            //密码正确则将iflogin字段设置为1
            if(rs->getString(4) == password) {
                ptcpUser->SetUserName(username);    //设置用户名
                sql = "UPDATE users SET iflogin=1  WHERE username=?";
                pstmt = conn->prepareStatement(sql);
                pstmt->setString(1, username);
                pstmt->executeUpdate();

                sql = "UPDATE users SET loginNum=?  WHERE username=?";
                pstmt = conn->prepareStatement(sql);
                pstmt->setInt(1,loginNum);
                pstmt->setString(2, username);
                pstmt->executeUpdate();

                root["typecode"] = Value(300);
                content = fastwriter.write(root);
            }
            //密码不正确
            else {
                root["typecode"] = Value(301);
                content = fastwriter.write(root);
            }
        }
        //找不到用户
        else {
                root["typecode"] = Value(302);
                content = fastwriter.write(root);
        }
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);
        ptcpUser->SendMsg(content);
    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

//使用usertel进行登录
void DBAnaysisData::TelLoginHandle(string &usertel, string &password) {
    try {
         Value root;
         FastWriter fastwriter;
         string content;
         PreparedStatement *pstmt;
         Connection *conn = pool->GetConnection();
         //conn->setAutoCommit(false);
         conn->setSchema(database);

         string sql = "SELECT * FROM users WHERE usertel=?";
         pstmt = conn->prepareStatement(sql);
         pstmt->setString(1, usertel);
         ResultSet *rs= pstmt->executeQuery();
         
         if(rs->next()){

             //cout<<rs->getString(1)<<endl;
             //cout<<rs->getString(2)<<endl;
             cout<<rs->getString(3)<<endl;
             int loginNum=rs->getInt(9);
             loginNum++;
             //判断密码是否正确
             if(rs->getString(4) == password) {
                 ptcpUser->SetUserTel(usertel);     //设置用户电话号码
                 sql = "UPDATE users SET iflogin=1 WHERE usertel=?";
                 pstmt = conn->prepareStatement(sql);
                 pstmt->setString(1, usertel);
                 pstmt->executeUpdate();

                sql = "UPDATE users SET loginNum=?  WHERE usertel=?";
                pstmt = conn->prepareStatement(sql);
                pstmt->setInt(1,loginNum);
                pstmt->setString(2, usertel);
                pstmt->executeUpdate();

                 root["typecode"] = Value(300);
                 content = fastwriter.write(root);
                 cout<<"login success"<<endl;
             }
             //密码不正确
             else {
                 root["typecode"] = Value(301);
                 cout<<"you are wrong"<<endl;
                 content = fastwriter.write(root);
             }
         }
         //找不到用户
         else {
             root["typecode"] = Value(302);
             content = fastwriter.write(root);
         }
         rs->close();
         delete pstmt;
         pool->ReleaseConnection(conn);
         //pool->DestroyConnection(conn);

         ptcpUser->SendMsg(content);
    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
       }
}

//临时用户使用未注册的usertel登录
void DBAnaysisData::TelNotLoginHandle(string &usertel) {
    try {
         Value root;
         FastWriter fastwriter;
         string content;
         PreparedStatement *pstmt;
         Connection *conn = pool->GetConnection();
         //conn->setAutoCommit(false);
         conn->setSchema("mysql");
         string sql = "SELECT * FROM users WHERE usertel=?";
         pstmt = conn->prepareStatement(sql);
         pstmt->setString(1, usertel);
         ResultSet *rs= pstmt->executeQuery();

         //如果bu能找到该用户
         if(!rs->next()){
            string sql = "INSERT INTO users(usertel, iflogin) values(?, ?)";
            pstmt= conn->prepareStatement(sql);
            pstmt->setString(1, usertel);
            pstmt->setInt(2, 3);

            //cout<<"sql:"<<sql<<endl;
            if(pstmt->executeUpdate() > 0) {

                cout<<"login2 success."<<endl;
                root["typecode"] = Value(510);
               // root["username"] = Value(username);
                content = fastwriter.write(root);
            }
            else {
                cout<<"login2 failed."<<endl;
                root["typecode"] = Value(514);
                content = fastwriter.write(root);
            }
         }

         else {
            //能找到用户，说明该号码已经备注测
             root["typecode"] = Value(513);
             content = fastwriter.write(root);
         }
         rs->close();
         delete pstmt;
         pool->ReleaseConnection(conn);
         //pool->DestroyConnection(conn);

         ptcpUser->SendMsg(content);
    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
       }
}

//username登出
void DBAnaysisData::NameLogoutHandle(string &username) {
    try {
        Value root;
        FastWriter fastwriter;
        string content;
        cout<<"NameLogoutHandle()"<<endl;
        Connection *conn = pool->GetConnection();
        //conn->setAutoCommit(false);
        conn->setSchema(database);
        PreparedStatement *pstmt;

        string sql = "SELECT * FROM users WHERE username=?";
        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, username);
        ResultSet *rs = pstmt->executeQuery();
        if(rs->next()){
            /*
            cout<<rs->getString(1)<<endl;
            cout<<rs->getString(2)<<endl;
            cout<<rs->getString(3)<<endl;
            cout<<rs->getString(4)<<endl;
            cout<<rs->getString(8)<<endl;
            */
            sql = "UPDATE users SET iflogin=0 WHERE username=?";
            pstmt = conn->prepareStatement(sql);
            pstmt->setString(1, username);
            pstmt->executeUpdate();
            root["typecode"] = Value(310);
            content = fastwriter.write(root);
        }
        else {
            root["typecode"] = Value(311);
            content = fastwriter.write(root);
        }
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);

        cout<<content<<endl;
        ptcpUser->SendMsg(content);

    } catch(SQLException &e) {
        string s_error(e.what());
        cout<<e.what()<<endl;
    }
}

//usertel登出
void DBAnaysisData::TelLogoutHandle(string &usertel) {
    try {
        Value root;
        FastWriter fastwriter;
        string content;
        cout<<"TelLogoutHandle()"<<endl;
        Connection *conn = pool->GetConnection();
        //conn->setAutoCommit(false);
        conn->setSchema(database);
        PreparedStatement *pstmt;

        string sql = "SELECT * FROM users WHERE usertel=?";
        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, usertel);
        ResultSet *rs = pstmt->executeQuery();
        if(rs->next()){
           /*
            cout<<rs->getString(1)<<endl;
            cout<<rs->getString(2)<<endl;
            cout<<rs->getString(3)<<endl;
            cout<<rs->getString(4)<<endl;
            cout<<rs->getString(8)<<endl;
            */
            sql = "UPDATE users SET iflogin=0 WHERE usertel=?";
            pstmt = conn->prepareStatement(sql);
            pstmt->setString(1, usertel);
            pstmt->executeUpdate();
            root["typecode"] = Value(310);
            content = fastwriter.write(root);
        }
        else {
            root["typecode"] = Value(311);
            content = fastwriter.write(root);
        }
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);

        cout<<content<<endl;
        ptcpUser->SendMsg(content);

    } catch(SQLException &e) {
        string s_error(e.what());
        cout<<e.what()<<endl;
    }
}


//定位，根据字段apid，返回所需结构体,AverageLocationPrint
Locate DBAnaysisData::LocationHandle(int &apid) {
    try {
         Value root;
         FastWriter fastwriter;
         string content;
         PreparedStatement *pstmt;
         Locate locate;
         Connection *conn = pool->GetConnection();
         //conn->setAutoCommit(false);
         conn->setSchema(database);

         string sql = "SELECT * FROM AverageLocationPrint WHERE apid=?";
         pstmt = conn->prepareStatement(sql);
         pstmt->setInt(1, apid);
         ResultSet *rs= pstmt->executeQuery();
         if(rs->next()){
             /*
			 cout<<rs->getString(1)<<endl;
			 cout<<rs->getInt(2)<<endl;
			 cout<<rs->getString(3)<<endl;
             cout<<rs->getString(4)<<endl;
             cout<<rs->getString(5)<<endl;
             */
             string s1 = rs->getString(1);
             int s2 = rs->getInt(2);
             float s3 = rs->getInt(3);
             float s4 = rs->getInt(4);
             string s5 = rs->getString(5);

             cout<<"s1 = "<<s1<<endl;
             cout<<"s2 = "<<s2<<endl;
             cout<<"s3 = "<<s3<<endl;
             cout<<"s4 = "<<s4<<endl;
             cout<<"s5 = "<<s5<<endl;

             locate.setter(s1, s2, s3, s4, s5);
             /*
             cout<<"locate-apid = "<<locate.getApid()<<endl;
			 root["typecode"] = Value(510);
			 root["locate_apid"] = Value(locate.getApid());
             content = fastwriter.write(root);
             */
        }
        //找不到用户
        else {
                root["typecode"] = Value(512);
                content = fastwriter.write(root);
        }
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);

        return locate;
        //ptcpUser->SendMsg(content);

    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

//定位2，根据字段apid，返回所需结构体,FingerPrint
vector<Locate> DBAnaysisData::Location2Handle(int &apid) {
    try {
         Value root;
         FastWriter fastwriter;
         string content;
         PreparedStatement *pstmt;
         vector<Locate> vect;
         Locate locate;
         int i=1;
         Connection *conn = pool->GetConnection();
         //conn->setAutoCommit(false);
         conn->setSchema(database);

         string sql = "SELECT * FROM FingerPrint WHERE apid=?";
         pstmt = conn->prepareStatement(sql);
         pstmt->setInt(1, apid);
         ResultSet *rs= pstmt->executeQuery();
         while(rs->next()){
             /*
			 cout<<rs->getString(1)<<endl;
			 cout<<rs->getInt(2)<<endl;
			 cout<<rs->getString(3)<<endl;
             cout<<rs->getString(4)<<endl;
             cout<<rs->getString(5)<<endl;
             */
             string s1 = rs->getString(1);
             int s2 = rs->getInt(2);
             float s3 = rs->getInt(3);
             float s4 = rs->getInt(4);
             string s5 = rs->getString(5);

             /*
             cout<<"第"<<i<<"个数据为："<<endl;
             cout<<"s1 = "<<s1<<endl;
             cout<<"s2 = "<<s2<<endl;
             cout<<"s3 = "<<s3<<endl;
             cout<<"s4 = "<<s4<<endl;
             cout<<"s5 = "<<s5<<endl;
             cout<<endl;
             i++;
             */

             locate.setter(s1, s2, s3, s4, s5);
             vect.push_back(locate);
             /*
             cout<<"locate-apid = "<<locate.getApid()<<endl;
			 root["typecode"] = Value(510);
			 root["locate_apid"] = Value(locate.getApid());
             content = fastwriter.write(root);
             */
        }
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);

        return vect;
        //ptcpUser->SendMsg(content);

    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

//用户历史位置信息存储
int DBAnaysisData::UserPositionsHandle(string &username, int positionX, int positionY) {
     try {
        int flag = false;
        Value root;
        string usertel;
        FastWriter fastwriter;
        string content;
        PreparedStatement *pstmt ;
        PreparedStatement *pstmt1;
        Connection *conn = pool->GetConnection();
        Connection *conn1 = pool->GetConnection();
        cout<<"UserPositionsHandle"<<endl;
        //conn->setAutoCommit(false);
        conn->setSchema("project");
        conn1->setSchema("project");
        string sql = "SELECT * FROM UserHistory WHERE username=? or usertel=?";
        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, username);
        pstmt->setString(2, username);
        ResultSet *rs= pstmt->executeQuery();
        //如果能查到此用户，证明已存在该用户的定位信息，继续添加历史位置信息
       
        int maxPath=0;
        int maxPathNum=0;
        int pathNum;
        int iflogin;
        int loginNum;
        int loginBeforNum;
        int path;
        int x,y;
        
        
        while(rs->next()){
            flag = true;
            if (rs->getInt(7)>=maxPath)
            {
                maxPath=rs->getInt(7);
            }

             path=maxPath;


        }
        //username select
        cout<<"maxPath="<<maxPath<<endl;
        string sql1 = "SELECT * FROM users WHERE username=? or usertel=?";
        pstmt1 = conn1->prepareStatement(sql1);
        pstmt1->setString(1,username);
        pstmt1->setString(2,username);
        rs=pstmt1->executeQuery();
        if(rs->next())
        {

            usertel=rs->getString(2);
            username=rs->getString(3);
            loginNum=rs->getInt(9);
            iflogin=rs->getInt(8);
          //  cout<<"iflogin="<<iflogin<<endl;
        }

         if(iflogin!=1)
         {
            if(iflogin==0)
             cout<<"please login"<<endl;
            else
                cout<<"please register"<<endl;


            rs->close();
            delete pstmt;
            delete pstmt1;
            pool->ReleaseConnection(conn);
            pool->ReleaseConnection(conn1);

            return -1;

         }
        //找不到用户，证明该用户是第一次进行定位历史记录,直接插入信息
        if(flag == false) {
            //cout<<"hml"<<endl;
            sql = "INSERT INTO UserHistory(uuid,usertel,username,positionX, positionY, path,pathNum,loginNum,date,time) VALUES(uuid(),?, ?, ?,?,?,?,?,NOW(),NOW())";
            
            pstmt = conn->prepareStatement(sql);
 

            pstmt->setString(1, usertel);
            pstmt->setString(2,username);
            pstmt->setInt(3, positionX);
            pstmt->setInt(4, positionY);
            pstmt->setInt(5, 1);
            pstmt->setInt(6,1);
            pstmt->setInt(7,loginNum);
            pstmt->executeUpdate();
            
            cout<<"creat a new user"<<endl;
            }
        else{
            //cout<<"hml"<<endl;
            sql="SELECT * FROM UserHistory WHERE path=? and username=?";
            pstmt1=conn1->prepareStatement(sql);
            pstmt1->setInt(1,path);
            pstmt1->setString(2,username);
            rs=pstmt1->executeQuery();
            while (rs->next())
            {
                if(rs->getInt(8)>=maxPathNum)
                {
                    maxPathNum=rs->getInt(8);
                    x=rs->getInt(4);
                    y=rs->getInt(5);
                    loginBeforNum=rs->getInt(9);

                 //   cout<<"maxPathNum="<<maxPathNum<<endl;
                }
           //     cout<<"username="<<rs->getString(3)<<endl;

            }
            int distance=sqrt((positionX-x)*(positionX-x)+(positionY-y)*(positionY-y));

            if(loginNum!=loginBeforNum)
            {
                path++;
                pathNum=1;
                distance=101;

           //     cout<<"1pathNum="<<pathNum<<endl;

            }
            else
            {
                pathNum=maxPathNum+1;
               // cout<<"2pathNum="<<pathNum<<endl;
            }
           if(distance>=100)
            {
                cout<<username<<endl;
                sql = "INSERT INTO UserHistory(uuid,usertel,username, positionX, positionY, path,pathNum,loginNum,date,time) VALUES(uuid(),?, ?, ?,?,?,?,?,NOW(),NOW())";
                pstmt = conn->prepareStatement(sql);
                pstmt->setString(1, usertel);
                pstmt->setString(2,username);
                pstmt->setInt(3, positionX);
                pstmt->setInt(4, positionY);
                pstmt->setInt(5, path);
                pstmt->setInt(6,pathNum);
                pstmt->setInt(7,loginNum);
                pstmt->executeUpdate();
                cout<<"succeed memory in previous record"<<endl;

            }
            else
            {

                cout<<"The distance is less,Don't memory"<<endl;


            }

        }

        rs->close();
        delete pstmt;
        delete pstmt1;
        pool->ReleaseConnection(conn);
        pool->ReleaseConnection(conn1);
 
        return 0;
    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}


int DBAnaysisData::GetHistory(string &username,int num,vector<string> &data,vector<vector<string> >&roadPosition){
try{
    bool flag = true;
    Value root;
   // string usertel;
    FastWriter fastwriter;
    string content;
    PreparedStatement *pstmt ;
  //  PreparedStatement *pstmt1;
    Connection *conn = pool->GetConnection();
   // Connection *conn1 = pool->GetConnection();
    cout<<"GetHistory"<<endl;

        //conn->setAutoCommit(false);
    conn->setSchema("project");
    // conn1->setSchema("project");

    string sql = "SELECT * FROM UserHistory WHERE username=? or usertel=?";
    pstmt = conn->prepareStatement(sql);
    pstmt->setString(1, username);
    pstmt->setString(2,username);
    ResultSet *rs= pstmt->executeQuery();
    vector<int> a;
    vector<int>::iterator it;
    while(rs->next())
    {
         for(it=a.begin();it!=a.end();it++)
         {
            if(rs->getInt(7)==*it)
                flag=false;
         }
         if(flag)
         {
            
            cout<<"getInt="<<rs->getInt(7)<<endl;
            a.push_back(rs->getInt(7));
         }
         flag=true;
         username=rs->getString(3);

    }

     sort(a.begin(),a.end(),greater<int>());
     for(it=a.begin();it!=a.end();it++)
     {
        cout<<"path="<<*it<<endl;
     }
     if(num >a.size())
     {
        cout<<"Don't have the "<<num<<" Hisrory"<<endl;

        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);


        return -1;
     }
     else
     {
        sql = "SELECT * FROM UserHistory WHERE username=? and path=?";
        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1,username);
        pstmt->setInt(2,a[num-1]);
        //cout<<"Lpath="<<a[num-1]<<endl;
        rs=pstmt->executeQuery();
        int maxPathNum;
        string endTime;
        stringstream roadPathOut;
        
        string strX;
        string strY;
        while(rs->next())
        {
            if(rs->getInt(8)==1)
            {
                data.push_back(rs->getString(10));
                data.push_back(rs->getString(11));
                cout<<"startTime="<<data[1]<<endl;
            }
            if(rs->getInt(8)>=maxPathNum)
            {
                maxPathNum=rs->getInt(8);
                endTime=rs->getString(11);
            }
            roadPathOut<<rs->getInt(4);
            strX=roadPathOut.str();
            vector<string>pathPosition;
            pathPosition.push_back(strX);
            roadPathOut.str("");
            strX=roadPathOut.str();
            roadPathOut<<rs->getInt(5);
            strY=roadPathOut.str();
            pathPosition.push_back(strY);
            roadPathOut.str("");;
            pathPosition.push_back(rs->getString(11));
            roadPosition.push_back(pathPosition);
            //pathPosition.clear();
           // roadPosition.push_back(Position);
       //     cout<<"HML"<<endl;
        }
        data.push_back(endTime);
        cout<<"endTime="<<data[2]<<endl;

        for(int i=0;i<roadPosition.size();i++)
        {
            
            for(int j=0;j<roadPosition[i].size();j++)
               {
                cout<<roadPosition[i][j]<<"  ";

               }
            cout<<endl;
        }



        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);

        return 0;
    }
  }catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }

}

//删除一条用户历史记录
int DBAnaysisData::deleteOneHistory(string &username,vector<string> &data){
try{
    Value root;
    FastWriter fastwriter;
    string content;
    cout<<"deleteOneHistory"<<endl;

    PreparedStatement *pstmt ;
    Connection *conn = pool->GetConnection();
    conn->setSchema("project");
    string StartTime=data[1];
    string sql = "SELECT * FROM UserHistory WHERE username=? and time=? or usertel=? and time=?";
    pstmt=conn->prepareStatement(sql);
    pstmt->setString(1, username);
    pstmt->setString(2,StartTime);
    pstmt->setString(3, username);
    pstmt->setString(4,StartTime);
    ResultSet *rs=pstmt->executeQuery();

    int path;
    while(rs->next())
    {
          path=rs->getInt(7);
          username=rs->getString(3);
    }
    sql="DELETE FROM UserHistory where username=? and path=?";
    pstmt=conn->prepareStatement(sql);
    pstmt->setString(1, username);
    pstmt->setInt(2,path);
    pstmt->executeUpdate();



    rs->close();
    delete pstmt;
    pool->ReleaseConnection(conn); 

    return 0;
   }catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

//删除用户全部历史记录
int DBAnaysisData::deleteAllHistory(string &username){
try{
    Value root;
    FastWriter fastwriter;
    string content;
    cout<<"deleteHistory"<<endl;

    PreparedStatement *pstmt ;
    Connection *conn = pool->GetConnection();
    conn->setSchema("project");

    string sql = "DELETE FROM UserHistory WHERE username=? or usertel=?";
    pstmt=conn->prepareStatement(sql);
    pstmt->setString(1, username);
    pstmt->setString(2, username);
    pstmt->executeUpdate();
   // rs->close();
    delete pstmt;
    pool->ReleaseConnection(conn); 

    return 0;
  }catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

