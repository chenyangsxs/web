//
// Created by chenyang on 21-8-22.
#include "CyWebserver.h"
CyWeb::CyWeb() {

    //root文件夹路径
    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

}
CyWeb::~CyWeb()
{
    close(m_epollfd);
    close(m_listenfd);
    delete[] m_http_users;
    delete m_pool;
}
void CyWeb::Init(int port,int threads_num,int max_requests)
{
    m_port = port;

    m_http_users = new http_conn[MAX_FD];//存放用户对象的数组
    assert(m_http_users);
    m_timer_users = new client_data[MAX_FD];
    m_pool = new threadpool<http_conn>(threads_num,max_requests);//线程池
    assert(m_pool);

    m_listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(m_listenfd >= 0);
}
void CyWeb::EpollListen()
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);
    int ret = 0;
    int flag = 1;
    setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));

    ret = bind(m_listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);//开始监听
    assert(ret >= 0);

    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);
    m_tool.addfd(m_epollfd,m_listenfd,false);//
    http_conn::m_epollfd = m_epollfd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    m_tool.setnonblocking(m_pipefd[1]);
    m_tool.addfd(m_epollfd, m_pipefd[0], false);//
    //添加信号
    addsig(SIGPIPE, SIG_IGN);
    addsig(SIGALRM, sig_handler);
    addsig(SIGTERM, sig_handler);

    alarm(TIMESLOT);//定时
    tools::m_epollfd = m_epollfd;
}
void CyWeb::KillTimer(util_timer *timer, int sockfd)
{//调用定时器装载的回调函数，执行删除相应连接在epoll上的注册，并且删除时间表上的计时器
    timer->cb_func(&m_timer_users[sockfd]);
    if (timer)
    {
        m_timerlist.del_timer(timer);
    }

}
void CyWeb::UserTimer(int connfd, struct sockaddr_in client_address) {
    m_http_users[connfd].init(connfd,client_address,m_root);
    //将listenfd监听到的链接请求的套接字储存在用户变量中m_timer_users
    m_timer_users[connfd].address = client_address;//
    m_timer_users[connfd].sockfd = connfd;//
    //对此连接的用户创建定时器
    util_timer *timer = new util_timer;
    timer->user_data = &m_timer_users[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;//当前时间后的3*5秒 计时器超时时间

    m_timer_users[connfd].timer = timer;//

    m_timerlist.add_timer(timer);
}
void CyWeb::DelayTimer(util_timer *timer) {
    time_t cur = time(NULL);
    timer->expire = cur + 3*TIMESLOT;
    m_timerlist.adjust_timer(timer);
}
void CyWeb::MainLogic() {
    //执行完成准备的函数 再执行完整逻辑函数
    bool timeout = false;
    bool stop_server = false;
    while(!stop_server)
    {
        int number = epoll_wait(m_epollfd,m_event,MAX_EVENT_NUMBER,-1);
        if (number < 0 && errno != EINTR)
        {
            printf("epoll failure");
            break;
        }
        for(int i = 0; i < number;++i)
        {
            int sockfd =  m_event[i].data.fd;
            if(sockfd == m_listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                while(1)//ET模式
                {
                    int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                    if(connfd < 0)break;
                    if(http_conn::m_user_count >= MAX_FD)
                    {
                        m_tool.showerror(connfd,"server busy");
                        break;
                    }
                    UserTimer(connfd,client_address);
                    std::printf("connect\n");
                }
            }
            else if(m_event[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                util_timer *timer = m_timer_users[sockfd].timer;
                KillTimer(timer, sockfd);
            }
            else if((sockfd == m_pipefd[0]) && (m_event[i].events & EPOLLIN))
            {
                int ret = 0;
                int sig;
                char signals[1024];
                ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
                if (ret == -1)
                {
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    for (int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                            case SIGALRM:
                            {
                                timeout = true;
                                break;
                            }
                            case SIGTERM:
                            {
                                stop_server = true;
                                break;
                            }
                        }
                    }
                }

            }
            else if(m_event[i].events & EPOLLIN)
            {
                //处理读事件
                util_timer *timer = m_timer_users[sockfd].timer;
                //采用模拟异步proactor的方式处理（感知io事件的读取完成状态，进而分配工作给线程）
                //实际上是同步模拟的效果
                if(m_http_users[sockfd].read_once())//读取完成的状态
                {
                    m_pool->append(m_http_users+sockfd);
                    if(timer)
                    {
                        DelayTimer(timer);
                    }
                }
                else
                {
                    KillTimer(timer,sockfd);
                }
            }
            else if(m_event[i].events & EPOLLOUT)//ET模式下写满write缓冲区触发
            {
                util_timer *timer = m_timer_users[sockfd].timer;
                if (m_http_users[sockfd].write())//写完成的状态
                {
                    if (timer)
                    {
                        DelayTimer(timer);
                    }
                }
                else
                {
                    KillTimer(timer, sockfd);
                }
            }
        }
        if(timeout)
        {
            m_timerlist.tick();
            alarm(TIMESLOT);
            timeout = false;
            std::printf("timeout\n");
        }
    }
}
void CyWeb::sig_handler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(m_pipefd[1], (char *)&msg, 1, 0);//向管道发送信号
    errno = save_errno;
}
void CyWeb::addsig(int sig,void(handler)(int)) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags |= SA_RESTART;//主进程被信号打断,SA_RESTART主进程处理完信号之后，自动重新执行主进程
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}
int CyWeb::m_pipefd[2];
sort_timer_lst CyWeb::m_timerlist;