//
// Created by chenyang on 21-8-16.
//

#ifndef CYWEBSERVER_H
#define CYWEBSERVER_H

#include "threadpool/threadpool.h"
#include "time/timer.h"
#include "http/http_conn.h"
#include <sys/socket.h>
#include <cassert>
#include "tools.h"
const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

class CyWeb{
public:
    CyWeb();
    ~CyWeb();

    void Init(int port,int threads_num,int max_requests);
    void EpollListen();
    void MainLogic();

private:
    char* m_root;

    threadpool<http_conn>* m_pool;
    epoll_event m_event[MAX_EVENT_NUMBER];
    http_conn* m_http_users;

    int m_port;
    int m_thread_num;
    int m_epollfd;
    int m_listenfd;

    //定时器没有添加

};
#endif //CYWEB_CYWEBSERVER_H
