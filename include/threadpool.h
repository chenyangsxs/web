/*
// Created by chenyang .
 半同步半反应堆线程池
    基础版本
    //mysql未添加
*/

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

template <typename T>
class threadpool{
public:
    threadpool(int thread_number,int max_requests);
    ~threadpool();
    bool append(T* request);//添加到任务队列

private:
    static void* worker(void* arg);//工作线程运行的函数，不断从任务队列取出任务
    void run();//执行

    /*参数列表*/
    int m_thread_number;        //线程池中最大线程数
    int m_max_requests;         //请求队列中的最大请求数
    pthread_t* m_threads;       //线程池中的线程数组
    std::list<T*> m_workqueue;  //请求任务队列
    locker m_queuelocker;       //保护请求队列的互斥锁
    sem m_queuestat;            //是否有任务需要处理
    bool m_stop;                //是否结束线程
};
template <typename T>
threadpool<T>::threadpool(int thread_number,int max_requests):m_thread_number(thread_number),m_max_requests(max_requests),m_stop(
        false),m_threads(NULL) {

    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();

    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
        throw std::exception();

    /*创建所有工作线程*/
    for( int i = 0; i < m_thread_number; ++i){
        if(pthread_create(m_threads + i,NULL,worker,this) != 0)
        {
            delete[] m_threads;
            throw  std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }

    }
}
template <typename T>
threadpool<T>::~threadpool() {
    delete[] m_threads;
    m_stop = true;
}
template <typename T>
bool threadpool<T>::append(T *request)
{
    /*往请求任务队列list m_workqueue中添加新打任务*/
    //多线程函数 需要上锁
    m_queuelocker.lock();
    if(m_workqueue.size() >= m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();//信号量加一
    return true;
}
template <typename T>
void *threadpool<T>::worker(void *arg)
{
    /*所有线程注册的函数，传入参数为同一个threadpool对象实体*/
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}
template <typename T>
void threadpool<T>::run()
{
    while(!m_stop)
    {
        m_queuestat.wait();//信号量减1
        m_queuelocker.lock();
        if (m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();//取出请求对象
        if (!request)
            continue;

        request->process();
    }
}
#endif //THREADPOOL_H
