#ifndef __CoTrain_MESSAGE_CPP__
#define __CoTrain_MESSAGE_CPP__

#include "message.h"
namespace CoTrain{

bool CoTrain::MessageQueue::start_on_threadpool(ThreadPool::ptr threadpool)
{
    threadpool->enqueue(
        //消息队列
        Task(1,
            [this](){
                while(1){
                    {
                        //线程安全
                        std::unique_lock<std::mutex> lock(this->m_mutex);
                        if(m_tcp_server != nullptr){
                            Message::ptr message = m_tcp_server->getMessage();
                            
                            if(message != nullptr){
                                //等待获取
                                this->m_sem_producer->wait();
                                this->m_message_queue.push(message);
                                this->m_sem_producer->notify();

                            }else if(m_stop){
                                //停止该服务
                                m_tcp_server->stop();
                                return;
                            }

                        }
                    }

                }
            }
        )
    );
    return false;
}

void CoTrain::MessageQueue::push(Message::ptr message)
{
    m_sem_producer->wait();
    m_message_queue.push(message);
    m_sem_producer->notify();
}

Message::ptr CoTrain::MessageQueue::pop()
{
    m_sem_producer->wait();
    if(!m_message_queue.empty()){
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            Message::ptr message = m_message_queue.front();
            m_message_queue.pop();
            m_sem_reduecer->notify();
            return message;
        }
    }else{
        return nullptr;
    }
}

}
#endif