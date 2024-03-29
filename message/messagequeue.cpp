#ifndef __CoTrain_MESSAGEQUEUE_CPP_
#define __CoTrain_MESSAGEQUEUE_CPP_
#include "messagequeue.h"

namespace CoTrain{

bool CoTrain::MessageQueue::start_on_threadpool(ThreadPool::ptr threadpool, uint32_t port)
{
    m_tcp_server->Listen(threadpool,port);
    threadpool->addLoopThread(
        //消息队列
        Thread::ptr(new Thread("MessageQueue4Com",
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
                            if(false){
                                
                            }
                            else{
                            Message::ptr message = m_tcp_server->getMessage(hasBufMessageToReceive());
                            
                            if(message != nullptr){
                                //等待获取
                                count++;
                                message->decodeHead();

                                if(message->getsize() > 0 && message->getType() == 0){
                                    this->subBufMessageToReceive();
                                }
                                
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
            }
        ))
    );

    // threadpool->addLoopThread(
    //     Thread::ptr(new Thread("MessageQueue4Buf",
    //         [this](){
    //             int count = 0;
    //             while(1){
    //                 {
    //                     // LogMannager::ptr logger = LogMannager::instance();
    //                     // logger->debug(
    //                     // logger->CreateEvent(
    //                     //     "This is messagequeue Thead"));
    //                     std::cout << "messagequeue here" << std::endl;
    //                     //线程安全
    //                     std::unique_lock<std::mutex> lock(this->m_mutex);
    //                     if(m_tcp_server != nullptr){


    //                         Message::ptr message = m_tcp_server->getMessage(true);
                            
    //                         if(message != nullptr){
    //                             //等待获取
    //                             count++;
    //                             this->push(message);
    //                         }else if(m_stop){
    //                             //停止该服务
    //                             m_tcp_server->stop();
    //                             m_sem_stop->notify();
    //                             return;
    //                         }else{
    //                             std::this_thread::sleep_for(std::chrono::seconds(1));
    //                         }

    //                     }
    //                 }
    //             }
    //         }
    //     ))
    // );
    // return false;
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
bool MessageQueue::hasBufMessageToReceive()
{
    std::unique_lock<std::mutex> lock(m_bufmessage4receive_mutex);
    if(m_bufmessage4receive_count > 0){
        // m_bufmessage4receive_count--;
        return true;
    }
    return false;
}
void MessageQueue::addBufMessageToReceive()
{
    std::unique_lock<std::mutex> lock(m_bufmessage4receive_mutex);
    m_bufmessage4receive_count++;
}
void MessageQueue::subBufMessageToReceive()
{
    std::unique_lock<std::mutex> lock(m_bufmessage4receive_mutex);
    m_bufmessage4receive_count--;
}
}

#endif