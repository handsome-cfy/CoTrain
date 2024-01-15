#ifndef __CoTrain_THREAD_CPP__
#define __CoTrain_THREAD_CPP__
#include "thread.h"

void CoTrain::Semaphore::notify()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_count++;
    m_condition.notify_one();
}

void CoTrain::Semaphore::wait()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    // 可以编译通过，不理他
    m_condition.wait(lock, [this]
                     { return m_count > 0; });
    m_count--;
}

void CoTrain::Semaphore::notify_all()
{
    m_condition.notify_all();
}

void CoTrain::ThreadPool::init()
{
    // 用于控制线程池的信号量
    m_pool_sem = Semaphore::ptr(new Semaphore(1));

    // 创建线程池
    for (int i = 0; i < m_max_thread_number; i++)
    {
        std::stringstream threadstring;
        threadstring << "PoolThread[" << i << "]";
        m_pool.push_back(
            Thread::ptr(new Thread(
                threadstring.str(), [this]()
                {
                    //此处是线程池内部单个线程的函数
                    while(true){
                        m_pool_sem->wait();
                        {
                        //获取任务队列的使用权
                        std::unique_lock<std::mutex> lock(m_task_mutex);
                        //获取到之后
                        if (!m_taskqueue.empty()){
                            Task task = m_taskqueue.top();
                            task.callback();
                            m_taskqueue.pop();
                        }
                        //如果任务队列为空，并且要求停止线程池，那么跳出循环
                        if(m_stop && m_taskqueue.empty())
                            return;
                        }
                        
                        m_pool_sem->notify();
                    } })));
    }
}

void CoTrain::ThreadPool::enqueue(const Task &task)
{
    {
        std::unique_lock<std::mutex> lock(m_task_mutex);
        m_taskqueue.push(task);
    }
    m_pool_sem->notify();
}

CoTrain::ThreadPool::~ThreadPool()
{
    {
    std::unique_lock<std::mutex> lock(m_task_mutex);
    m_stop = true;
    }
    m_pool_sem->notify_all();
    for(auto& thread : m_pool){
        thread->join();
    }
}

#endif
