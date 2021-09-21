//
// Created by chenyang on 21-8-22.
//

#ifndef CYWEB_TOOLS_H
#define CYWEB_TOOLS_H

#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <string.h>
#include <map>
#include <string>
#include <assert.h>
class tools
{
public:
    tools() {}
    ~tools() {}

    //void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot);

    //信号处理函数
    //static void sig_handler(int sig);

    //设置信号函数
    //void addsig(int sig, void(handler)(int), bool restart = true);

    //void show_error(int connfd, const char *info);

    //发送错误信息
    void showerror(int connfd, const char *info);
    static int m_epollfd;
};

#endif //CYWEB_TOOLS_H
