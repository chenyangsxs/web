//
// Created by chenyang on 21-9-16.
//
#include <mysql/mysql.h>
#include <string>
#include "sqlconnect.h"
Sqlconnect::Sqlconnect() {

}
Sqlconnect::~Sqlconnect() {

}
Sqlconnect* Sqlconnect::GetInstance() {
    static Sqlconnect m_sqlcon;
    return &m_sqlcon;
}
void Sqlconnect::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn){
    m_url = url;
    m_Port = Port;
    m_User = User;
    m_PassWord = PassWord;
    m_DatabaseName = DBName;

    for (int i = 0; i < MaxConn; i++)
    {
        MYSQL *con = NULL;
        con = mysql_init(con);

        if (con == NULL)
        {

            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

        if (con == NULL)
        {

            exit(1);
        }
        m_sqlconlist.push_back(con);
        ++m_FreeConn;
        reserve = sem(m_FreeConn);
        m_MaxConn = m_FreeConn;
    }
}
MYSQL* Sqlconnect::GetOneConnect(){
    MYSQL *con = NULL;
    if (0 == m_sqlconlist.size())
        return NULL;
    reserve.wait();
    lock.lock();
    con = m_sqlconlist.front();
    m_sqlconlist.pop_front();
    --m_FreeConn;
    ++m_CurConn;
    lock.unlock();
    return con;
}
//释放当前使用的连接
bool Sqlconnect::ReleaseConnection(MYSQL *con)
{
    if (NULL == con)
        return false;

    lock.lock();

    m_sqlconlist.push_back(con);
    ++m_FreeConn;
    --m_CurConn;

    lock.unlock();

    reserve.post();
    return true;
}

//销毁数据库连接池
void Sqlconnect::DestroyPool()
{

    lock.lock();
    if (m_sqlconlist.size() > 0)
    {
        list<MYSQL *>::iterator it;
        for (it = m_sqlconlist.begin(); it != m_sqlconlist.end(); ++it)
        {
            MYSQL *con = *it;
            mysql_close(con);
        }
        m_CurConn = 0;
        m_FreeConn = 0;
        m_sqlconlist.clear();
    }
    lock.unlock();
}