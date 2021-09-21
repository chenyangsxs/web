//
// Created by chenyang on 21-8-16.
//

#ifndef CYWEBSERVER_H
#define CYWEBSERVER_H

#include <sys/socket.h>
#include <cassert>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include "threadpool.h"
#include "timer.h"
#include "http_conn.h"
#include "tools.h"
#include "sqlconnect.h"
const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位
struct SqlLog{
    string m_user, m_passWord, m_databaseName;
    int m_sql_num;
};
class CyWeb{
public:
    CyWeb();
    ~CyWeb();
    void Init(int port,int threads_num,int max_requests);
    void PrepareSql();
    void EpollListen();
    void MainLogic();
    tools m_tool;//工具函数
private:
    char* m_root;

    threadpool<http_conn>* m_pool;
    epoll_event m_event[MAX_EVENT_NUMBER];
    http_conn* m_http_users;//user的http对象
    client_data* m_timer_users;//user 对象 包含连接地址， socket，timer

    int m_port;
    SqlLog m_Sql;
    int m_thread_num;
    int m_epollfd;
    int m_listenfd;
    bool stop_server;
    //定时器
    static int m_pipefd[2];//传输信号的管道 0写入 1读出
    static sort_timer_lst m_timerlist;//升序链表

    static void sig_handler(int sig);//信号处理函数
    void addsig(int sig,void(handler)(int));

    void UserTimer(int connfd , struct sockaddr_in client_address);
    void KillTimer(util_timer *timer, int sockfd);
    void DelayTimer(util_timer *timer);
    //void TimerHandler();
};
#endif //CYWEB_CYWEBSERVER_H
