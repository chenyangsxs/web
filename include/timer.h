//
// Created by chenyang on 21-8-22.
//

#ifndef CYWEB_TIMER_H
#define CYWEB_TIMER_H

#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include "http_conn.h"
#include "tools.h"

class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire;//任务的超时时间

    void (* cb_func)(client_data *);
    client_data *user_data;
    util_timer *prev;
    util_timer *next;
};

class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();

    void add_timer(util_timer *timer);
    void adjust_timer(util_timer *timer);
    void del_timer(util_timer *timer);
    void tick();

private:
    void add_timer(util_timer *timer, util_timer *lst_head);

    util_timer *head;
    util_timer *tail;
};
void cb_func(client_data *user_data);
#endif //CYWEB_TIMER_H
