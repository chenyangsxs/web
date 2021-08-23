//
// Created by chenyang on 21-8-16.
//

#ifndef CYWEB_HTTP_CONN_H
#define CYWEB_HTTP_CONN_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>

#include "../lock/locker.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;//文件名最大长度
    static const int READ_BUFFER_SIZE = 2048;//读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;//写缓冲区大小
    /*http请求方法*/
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    /*主状态机*/
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    /*返回码*/
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    /*从状态机*/
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    void init(int sockfd, const sockaddr_in &addr);
    void close_conn(bool real_close = true);
    void process();//处理客户请求
    bool read_once();
    bool write();
    sockaddr_in *get_address()
    {
        return &m_address;
    }

private:
    void init();
    HTTP_CODE process_read();
    /*填充http应答*/
    bool process_write(HTTP_CODE ret);
    /*下面一组函数被process_read调用*/
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();
    /*下面一组函数被process_write调用*/
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    int m_state;  //读为0, 写为1

private:
    /*请求链接的socket 以及 对方的地址*/
    int m_sockfd;
    sockaddr_in m_address;
    /*读缓冲区*/
    char m_read_buf[READ_BUFFER_SIZE];
    /*读缓冲区已经读入的数据的最后字节的下一个位置*/
    int m_read_idx;
    /*当前正在分析打字符在读缓冲区的位置*/
    int m_checked_idx;
    /*当前解析行的起始位置*/
    int m_start_line;
    /*写缓冲区*/
    char m_write_buf[WRITE_BUFFER_SIZE];
    /*写缓冲区待发送打字节数*/
    int m_write_idx;

    /*主状态机所处状态*/
    CHECK_STATE m_check_state;
    /*请求类型方法*/
    METHOD m_method;
    /*客户请求的文件类型的完整路径，等于doc_root+m_url*/
    char m_real_file[FILENAME_LEN];
    /*请求文件的文件名*/
    char *m_url;
    /*http协议版本号 支持http1.1*/
    char *m_version;
    /*主机名字*/
    char *m_host;
    /*http请求消息长度*/
    int m_content_length;
    /*http请求是否保持连接*/
    bool m_linger;

    /*客户请求的目标文件被mmap到内存中的起始位置*/
    char *m_file_address;
    /*目标文件状态，通过它判断文件是否存在、是否为目录、是否刻可读，并获取文件的大小信息*/
    struct stat m_file_stat;

    /*writev执行写操作所需要的参数*/
    struct iovec m_iv[2];
    int m_iv_count;

    int bytes_to_send;
    int bytes_have_send;
    char *doc_root;

};


#endif //CYWEB_HTTP_CONN_H
