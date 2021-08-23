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
CyWeb::~CyWeb() {
    close(m_epollfd);
    close(m_listenfd);
    delete[] m_http_users;
    delete m_pool;
}
void CyWeb::Init(int port,int threads_num,int max_requests) {
    m_port = port;

    m_http_users = new http_conn[MAX_FD];
    assert(m_http_users);

    m_pool = new threadpool<http_conn>(threads_num,max_requests);
    assert(m_pool);
}
void CyWeb::EpollListen() {
    
}