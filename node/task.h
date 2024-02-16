#ifndef __CoTrain_TASK_H__
#define __CoTrain_TASK_H__

#include"../thread.h"
#include"../message/message.h"
#include"../message/uniqueid.h"
#include"node.h"

namespace CoTrain{

void AliveTask(ComMessage::ptr message){

}

// void MessageQueue::process_com_message(ThreadPool::ptr threadpool)
// {
//     threadpool->addLoopThread(
//         Thread::ptr(new Thread("Reducer",
//             [this, threadpool]()
//             {
//                 // Semaphore::ptr producer = messagequeue->getsemproducer();
//                 // Semaphore::ptr reducer = messagequeue->getsemreduecer();
//                 LogManager::ptr logger = LogManager::instance();
//                 //
                
//                 while (true)
//                 {
//                     // reducer->wait();
//                     ComMessage::ptr message = this->pop();
//                     // producer->notify();

//                     // std::cout << "this is reducer" << endl;
//                     logger->debug(
//                         logger->CreateEvent(
//                             std::string((char *)(message->getdata()),message->getsize())));

//                     message->decodeHead();

//                     logger->debug(
//                         logger->CreateEvent(
//                             message->showheader()
//                         )
//                     );
                    
//                 }
//             })));
// }

class TaskFactory{
public:
    typedef std::shared_ptr<TaskFactory> ptr;
    
    TaskFactory(){}

    // 对于服务端而言，这里是获取到ComMessage之后产生对应的Task，用来处理
    void MakeTask(Message::ptr message, ServerNode::ptr servernode);    
    // 重载函数，专门用来处理与socket相关的任务
    // void MakeTask(ComMessage::ptr message, ServerNode::ptr servernode, TcpSocket::ptr socket);

    void ProcessMessage(ServerNode::ptr servernode);

    bool getstop();
    void setstop(bool val);
private:
    // 是否结束
    std::mutex m_stop_mutex;
    bool m_stop = false;

};

}

#endif