#ifndef __CoTrain_MESSAGEQUEUE_H__
#define __CoTrain_MESSAGEQUEUE_H__
#include"../socket.h"
#include"message.h"
namespace CoTrain{

class MessageQueue{
public:
    typedef std::shared_ptr<MessageQueue> ptr;

    //把消息队列机制创建在线程池子上
    bool start_on_threadpool(ThreadPool::ptr threadpool);

    void push(Message::ptr message);
    Message::ptr pop();
        

private:
    //互斥锁
    std::mutex m_mutex;
    //消息队列
    std::queue<Message::ptr> m_message_queue;
    //用于控制消息队列中存在的数据量的信号量
    Semaphore::ptr m_sem_producer = Semaphore::ptr(new Semaphore(m_len));
    Semaphore::ptr m_sem_reduecer = Semaphore::ptr(new Semaphore(0));
    //current_len
    uint16_t m_len = 0;
    //server
    TcpServer::ptr m_tcp_server;
    //wether stop
    bool m_stop = false;
};
}
#endif