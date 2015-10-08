#include <define.h>
#include <Database.h>
#include <sstream>

using namespace std;



const char *database = "project";
ConnPool* Database::pool = ConnPool::GetInstance();
//使用用户名或电话注册操作
int  Database::RegisterHandle(string &usertel, string &username, string &password) {
    try {
        bool flag1 = false;
        bool flag2 = false;
        int typecode;

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
            cout<<"register failed"<<endl;
            if(flag1==true&&flag2==true)
                cout<<"usename and usetel is repeated"<<endl;
            else if(flag1==true)
                cout<<"usertel is repeated"<<endl;
            else 
                cout<<"username is repeated"<<endl;
                typecode=501;

           // content = fastwriter.write(root);
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
                typecode=500;
               // root["username"] = Value(username);
               
            }
            else {
                cout<<"register failed2."<<endl;
                cout<<"information insert is failed"<<endl;
                 typecode=501;
               
            }
        }
        delete pstmt;
        pool->ReleaseConnection(conn);
        return typecode;
        //pool->DestroyConnection(conn);
       // cout<<content<<endl;
        

    }
    catch(SQLException &e) {
        string s_error(e.what());
        cout<<e.what()<<endl;
    }
}

//username登录操作
int  Database::NameLoginHandle(string &username, string &password) {
    try {

 
        int typecode;
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
                //ptcpUser->SetUserName(username);    //设置用户名
                sql = "UPDATE users SET iflogin=1  WHERE username=?";
                pstmt = conn->prepareStatement(sql);
                pstmt->setString(1, username);
                pstmt->executeUpdate();

                sql = "UPDATE users SET loginNum=?  WHERE username=?";
                pstmt = conn->prepareStatement(sql);
                pstmt->setInt(1,loginNum);
                pstmt->setString(2, username);
                pstmt->executeUpdate();

                cout<<"namelogin is success"<<endl;
                typecode=510;
       
               // content = fastwriter.write(root);
            }
            //密码不正确
            else {
                cout<<"password is wrong"<<endl;
                typecode=511;
               // content = fastwriter.write(root);
            }
        }
        //找不到用户
        else {
                cout<<"usename is wrong"<<endl;
                 typecode=512;
                //content = fastwriter.write(root);
        }
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);
        return typecode;
        //pool->DestroyConnection(conn);
        //ptcpUser->SendMsg(content);
    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

//使用usertel进行登录
int  Database::TelLoginHandle(string &usertel, string &password) {
    try {
        int typecode;
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
                // ptcpUser->SetUserTel(usertel);     //设置用户电话号码
                 sql = "UPDATE users SET iflogin=1 WHERE usertel=?";
                 pstmt = conn->prepareStatement(sql);
                 pstmt->setString(1, usertel);
                 pstmt->executeUpdate();

                sql = "UPDATE users SET loginNum=?  WHERE usertel=?";
                pstmt = conn->prepareStatement(sql);
                pstmt->setInt(1,loginNum);
                pstmt->setString(2, usertel);
                pstmt->executeUpdate();

                 typecode=510;
                // content = fastwriter.write(root);
                 cout<<"tellogin success"<<endl;
             }
             //密码不正确
             else {
                cout<<"password is wrong"<<endl;
               typecode=511;
                 cout<<"you are wrong"<<endl;
                 //content = fastwriter.write(root);
             }
         }
         //找不到用户
         else {
             cout<<"usetel is wrong"<<endl;
             typecode=512;
            // content = fastwriter.write(root);
         }
         rs->close();
         delete pstmt;
         pool->ReleaseConnection(conn);
         //pool->DestroyConnection(conn);

        // ptcpUser->SendMsg(content);
         return typecode;
    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
       }
}

//临时用户使用未注册的usertel登录
int  Database::TelNotLoginHandle(string &usertel) {
    try {
         int typecode;
         PreparedStatement *pstmt;
         Connection *conn = pool->GetConnection();
         //conn->setAutoCommit(false);
         conn->setSchema(database);
         string sql = "SELECT * FROM users WHERE usertel=?";
         pstmt = conn->prepareStatement(sql);
         pstmt->setString(1, usertel);
         ResultSet *rs= pstmt->executeQuery();

         //如果不能找到该用户
         if(!rs->next()){
            string sql = "INSERT INTO users(usertel, iflogin) values(?, ?)";
            pstmt= conn->prepareStatement(sql);
            pstmt->setString(1, usertel);
            pstmt->setInt(2, 3);

            //cout<<"sql:"<<sql<<endl;
            if(pstmt->executeUpdate() > 0) {

                cout<<"tempory login success."<<endl;
     
                typecode=510;
               // root["username"] = Value(username);
               // content = fastwriter.write(root);
            }
            else {
                cout<<" rempory login failed."<<endl;
         
                typecode =514;
                //content = fastwriter.write(root);
            }
         }

         else {
            //能找到用户，说明该号码已经备注测
             cout<<"usertel is register"<<endl;
         
             typecode=513;
             //content = fastwriter.write(root);
         }
         rs->close();
         delete pstmt;
         pool->ReleaseConnection(conn);
         return typecode;
         //pool->DestroyConnection(conn);

         //ptcpUser->SendMsg(content);
    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
       }
}

//username登出
int  Database::NameLogoutHandle(string &username) {
    try {

        
        int typecode;        
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
           
            typecode=310;
            cout<<"name logout success"<<endl;
        }
        else {
            cout<<"name logout failed"<<endl;
            typecode=311;
         
        }
        rs->close();
        delete pstmt;

        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);

       
      return typecode;

    } catch(SQLException &e) {
        string s_error(e.what());
        cout<<e.what()<<endl;
    }
}

//usertel登出
int   Database::TelLogoutHandle(string &usertel) {
    try {

        int typecode;
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
            typecode=310;

            cout<<"tel logout success"<<endl;
        }
        else {
            cout<<"tel logout failed"<<endl;
              typecode=310;
           }
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);

       return typecode;
       // ptcpUser->SendMsg(content);

    } catch(SQLException &e) {
        string s_error(e.what());
        cout<<e.what()<<endl;
    }
}




//定位2，根据字段apid，返回所需结构体,FingerPrint
int  Database::LocationHandleData(int &apid,vector<Locate> &vect) {
    try {
        int typecode;

         PreparedStatement *pstmt;
         Locate locate;
         int i=1;
         Connection *conn = pool->GetConnection();
         //conn->setAutoCommit(false);
         conn->setSchema(database);

         string sql = "SELECT * FROM FingerPrint WHERE apid=?";
    
         pstmt = conn->prepareStatement(sql);
         pstmt->setInt(1, apid);
         ResultSet *rs= pstmt->executeQuery();
         int flag=0;
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
             flag=1;
        }
        if(flag==1)
            typecode=1540;
        else
          typecode=1541;
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);
        //pool->DestroyConnection(conn);

        return typecode;
        //ptcpUser->SendMsg(content);

    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

int  Database::GuideHandleData(map<int,node > &nodeMap,map<int,edge> &edgeMap)
{   
try{
  
    
    int typecode;
    Connection *conn = pool->GetConnection();
    conn->setSchema(database);
    string sql = "SELECT * FROM node";
    PreparedStatement *pstmt ;
    pstmt = conn->prepareStatement(sql);
  
    ResultSet *rs= pstmt->executeQuery();
     node v;
     int flag1=0,flag2=0;
     int nodeId;
    while(rs->next())
    {
       
        v.nodename=rs->getString(5);
        v.position_x=rs->getInt(2);
        v.position_y=rs->getInt(3);
        v.m_index=rs->getInt(1);
        nodeId=rs->getInt(1);
        nodeMap.insert(make_pair(nodeId,v));
        flag1=1;
    }
    
    sql="select * from edge ";
   
    pstmt=conn->prepareStatement(sql);
   
    rs=pstmt->executeQuery();
    edge vg;
    int edgeId;
    while(rs->next())
    {
       vg.edgeid=rs->getInt(1);
       vg.frontnodeid=rs->getInt(2);
       vg.lastnodeid=rs->getInt(3);
       vg.edgeweightid=rs->getInt(4);
       edgeId=rs->getInt(1);
       edgeMap.insert(make_pair(edgeId,vg));
       flag2=1;
    }
    if(flag1==1&&flag2==1)
       typecode=1510;
    else
        typecode=1511;
     delete pstmt;
     pool->ReleaseConnection(conn);
     return typecode;

  }
    


catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

//用户历史位置信息存储
int Database::UserPositionsHandle(string &username, int positionX, int positionY) {
     try {
        int flag = false;
        int typecode;
        string usertel;
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
            typecode=401;
            rs->close();
        delete pstmt;
        delete pstmt1;
        pool->ReleaseConnection(conn);
        pool->ReleaseConnection(conn1);
 
        return typecode;

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
           
            if( pstmt->executeUpdate()>0)
            {
              cout<<"creat a new userHistory"<<endl;
              typecode = 400;
            }

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
                typecode=402;

            }
            else
            {

                cout<<"The distance is less,Don't memory"<<endl;
                  typecode=403;


            }

        }

        rs->close();
        delete pstmt;
        delete pstmt1;
        pool->ReleaseConnection(conn);
        pool->ReleaseConnection(conn1);
 
        return typecode;
    } catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}


int  Database::GetHistory(string &username,int num,// 其中,data用来存储每条路径的日期,开始时间与结束时间

    vector<string> &data,vector<vector<string> >&roadPosition){//roadPosition用于存储用户的路径(该路径包含多个位置),位置格式为[“x”:1, ”y”:2, “time”:”13:45”]
try{
    bool flag = true;

   // string usertel;
    int typecode;

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

        typecode=1561;
        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);


        return typecode;
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
            roadPathOut.str("");
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


       typecode=1560;

        rs->close();
        delete pstmt;
        pool->ReleaseConnection(conn);

        return typecode;
    }
  }catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }

}

//删除一条用户历史记录
int  Database::deleteOneHistory(string &username,vector<string> &data){
try{
    int typecode;
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
    int i=pstmt->executeUpdate();
    if(i<0)
        typecode=1576;
    else
      typecode=1575;


    rs->close();
    delete pstmt;
    pool->ReleaseConnection(conn); 

    return typecode;
   }catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

//删除用户全部历史记录
int Database::deleteAllHistory(string &username){
try{
    int typecode;
    cout<<"deleteHistory"<<endl;

    PreparedStatement *pstmt ;
    Connection *conn = pool->GetConnection();
    conn->setSchema("project");

    string sql = "DELETE FROM UserHistory WHERE username=? or usertel=?";
    pstmt=conn->prepareStatement(sql);
    pstmt->setString(1, username);
    pstmt->setString(2, username);
    int i=pstmt->executeUpdate();
   // rs->close();
    if(i<0)
         typecode=1571;
    else
      typecode=1570;

    delete pstmt;
    pool->ReleaseConnection(conn); 

    return typecode;
  }catch (SQLException &e) {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
    }
}

