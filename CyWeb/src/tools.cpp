//
// Created by chenyang on 21-8-22.
//

#include "tools.h"
int tools::setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
void tools::addfd(int epollfd, int fd, bool one_shot)
{
    epoll_event event;
    event.data.fd = fd;


    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;


    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}
//设置信号函数
//void tools::addsig(int sig, void(handler)(int), bool restart)
//{
//    struct sigaction sa;
//    memset(&sa, '\0', sizeof(sa));
//    sa.sa_handler = handler;
//    if (restart)
//        sa.sa_flags |= SA_RESTART;
//    sigfillset(&sa.sa_mask);
//    assert(sigaction(sig, &sa, NULL) != -1);
//}
//
void tools::showerror(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}
int tools::m_epollfd;