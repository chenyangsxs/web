//
// Created by chenyang on 21-9-16.
//

#ifndef CYWEB_SQLCONNECT_H
#define CYWEB_SQLCONNECT_H
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string>
#include <list>
#include "locker.h"

using namespace std;
class Sqlconnect{
public:
    MYSQL* GetOneConnect();
    bool ReleaseConnection(MYSQL *conn); //释放连接
    //int  GetFreeConn();					 //获取连接
    void DestroyPool();					 //销毁所有连接
    void init(string url, string User, string PassWord, string DBName, int Port, int MaxConn);
    static Sqlconnect* GetInstance();//
private:
    Sqlconnect();
    ~Sqlconnect();
    int m_MaxConn;  //最大连接数
    int m_CurConn;  //当前已使用的连接数
    int m_FreeConn; //当前空闲的连接数
    locker lock;
    list<MYSQL*> m_sqlconlist;
    sem reserve;
public:
    string m_url;			 //主机地址
    string m_Port;		 //数据库端口号
    string m_User;		 //登陆数据库用户名
    string m_PassWord;	 //登陆数据库密码
    string m_DatabaseName; //使用数据库名
};

#endif //CYWEB_SQLCONNECT_H
