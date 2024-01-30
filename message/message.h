#ifndef __CoTrain_MESSAGE_H__
#define __CoTrain_MESSAGE_H__

#include<memory>
#include<string>
#include"../log.h"
#include"../thread.h"
#include"../socket.h"

#include"uniqueid.h"

namespace CoTrain{


class Message{
public:
    typedef std::shared_ptr<Message> ptr;

    void* getdata(){return m_data;}
    void setdata(void* val){m_data = val;}

    void setID(UniqueID::ptr val){m_uniqueid = val;}
    UniqueID::ptr getID(){return m_uniqueid;}

    Message(){}
    //对于不同的message，使用不同的m——data释放策略
    virtual ~Message() = 0;
    virtual void* getdata_withid() = 0;

protected:
    void * m_data;
    UniqueID::ptr m_uniqueid;
    constexpr static uint16_t max_buf_len = 1500;

};
//  带有一个缓冲区的message，设计用来传递数据
class BufMessage: public Message{

};

//  用于构建分布式系统的message
class ComMessage: public Message{
public:

private:
};

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