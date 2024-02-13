#ifndef __CoTrain_MESSAGEQUEUE_CPP_
#define __CoTrain_MESSAGEQUEUE_CPP_
#include "messagequeue.h"

namespace CoTrain{

bool CoTrain::MessageQueue::start_on_threadpool(ThreadPool::ptr threadpool, uint32_t port)
{
    m_tcp_server->Listen(threadpool,port);
    threadpool->addLoopThread(
        //消息队列
        Thread::ptr(new Thread("MessageQueue",
            [this](){
                int count = 0;
                while(1){
                    {
                        // LogMannager::ptr logger = LogMannager::instance();
                        // logger->debug(
                        // logger->CreateEvent(
                        //     "This is messagequeue Thead"));
                        std::cout << "messagequeue here" << std::endl;
                        //线程安全
                        std::unique_lock<std::mutex> lock(this->m_mutex);
                        if(m_tcp_server != nullptr){
                            Message::ptr message = m_tcp_server->getMessage();
                            
                            if(message != nullptr){
                                //等待获取
                                count++;
                                this->push(message);
                            }else if(m_stop){
                                //停止该服务
                                m_tcp_server->stop();
                                m_sem_stop->notify();
                                return;
                            }else{
                                std::this_thread::sleep_for(std::chrono::seconds(1));
                            }

                        }
                    }

                }
            }
        ))
    );
    return false;
}

void CoTrain::MessageQueue::push(Message::ptr message)
{
    m_sem_producer->wait();
    m_message_queue.push(message);
    m_sem_reduecer->notify();
}

Message::ptr CoTrain::MessageQueue::pop()
{
    m_sem_reduecer->wait();
    // std::unique_lock<std::mutex> lock(m_mutex);
    if(!m_message_queue.empty()){
        {
            Message::ptr message = m_message_queue.front();
            m_message_queue.pop();
            m_sem_producer->notify();
            return message;
        }
    }else{
        return nullptr;
    }
}

MessageQueue::~MessageQueue()
{
    // m_tcp_server->stop();
    m_stop = true;
    m_sem_stop->wait();
}
MessageQueue::MessageQueue()
{
    m_tcp_server = TcpServer::ptr(new TcpServer());
}
MessageQueue::MessageQueue(ServerNodeConfig::ptr config)
{
    //TODO MessageQueue Config
    m_tcp_server = TcpServer::ptr(new TcpServer(config));
}
}

#endif