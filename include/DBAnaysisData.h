#ifndef ANAYSIS_DATA_H
#define ANAYSIS_DATA_H

#include <TcpUser.h>
#include <Locate.h>

#include <vector>
using std::vector;

#include <string>
using std::string;


class DBAnaysisData {
   
    public:
        explicit DBAnaysisData(TcpUser* ptcpUser):ptcpUser(ptcpUser){}
        ~DBAnaysisData(){};
        void Handle(const string &content);
        void UseHandle(string &usertel);
        void RegisterHandle(string &usertel, string &username, string &password);
        void NameLoginHandle(string &username, string &password);
        void TelLoginHandle(string &usertel, string &password);
        void TelNotLoginHandle(string &usertel);
        void NameLogoutHandle(string &username);
        void TelLogoutHandle(string &usertel);
        Locate LocationHandle(int &apid);
        static vector<Locate> Location2Handle(int &apid);
        void SendStr(string &content);

        int UserPositionsHandle(string &username, int positionX, int positionY);
        int GetHistory(string &username,int num,vector<string> &data,vector<vector<string> >&roadPosition);
        int deleteOneHistory(string &username,vector<string> &data);
        int deleteAllHistory(string &username);

        static void test();
        static void loadMap2();

    private:
        TcpUser *ptcpUser;
};
#endif


