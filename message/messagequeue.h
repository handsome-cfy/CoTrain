#ifndef __CoTrain_MESSAGEQUEUE_H__
#define __CoTrain_MESSAGEQUEUE_H__
#include"../socket.h"
#include"message.h"
#include"../config/config.h"
namespace CoTrain{

class MessageQueue{
public:
    typedef std::shared_ptr<MessageQueue> ptr;

    //表明这个messagequeue是接受哪种信息的
    enum Type{
        Com,
        Buf
    };

    //把消息队列机制创建在线程池子上
    bool start_on_threadpool(ThreadPool::ptr threadpool, uint32_t port);

    // void process_com_message(ThreadPool::ptr threadpool);

    void push(Message::ptr message);
    Message::ptr pop();

    Semaphore::ptr getsemproducer(){return m_sem_producer;}
    Semaphore::ptr getsemreduecer(){return m_sem_reduecer;}
        
    ~MessageQueue();

    MessageQueue();
    MessageQueue(ServerNodeConfig::ptr config);

    bool hasBufMessageToReceive();
    void addBufMessageToReceive();
    void subBufMessageToReceive();

    uint16_t getsize(){return m_message_queue.size();}
private:
    //互斥锁
    std::mutex m_mutex;
    //消息队列
    std::queue<Message::ptr> m_message_queue;

    
    //current_len
    uint16_t m_len = 0;

    //用于控制消息队列中存在的数据量的信号量
    Semaphore::ptr m_sem_producer = Semaphore::ptr(new Semaphore(Max_Producer_Number));
    Semaphore::ptr m_sem_reduecer = Semaphore::ptr(new Semaphore(0));

    //server
    TcpServer::ptr m_tcp_server;
    //wether stop
    bool m_stop = false;
    Semaphore::ptr m_sem_stop = Semaphore::ptr(new Semaphore(0));

    //用来记录有几个buf文件需要接受
    std::mutex m_bufmessage4receive_mutex;
    uint16_t m_bufmessage4receive_count = 0;

    static const uint16_t Max_Producer_Number = 20;
};
}
#endif