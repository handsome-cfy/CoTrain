#include "../socket.h"
#include "../message/messagequeue.h"
#include "../node/node.h"
#include <iostream>
using namespace std;
using namespace CoTrain;

int main(void)
{
    // 初始化线程池
    ThreadPool::ptr pool = ThreadPool::ptr(new ThreadPool());
    pool->init();

    TcpServer::ptr server = TcpServer::ptr(new TcpServer());
    // server->Listen(8000);

    LogManager::ptr logger = LogManager::instance();
    // for(int i = 0; i < 30; i++){
    //     pool->enqueue(std::move<Task>(Task(5,[i,logger](){
    //     {
    //     // std::unique_lock<std::mutex>lock(log_mutex) ;
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     std::stringstream ss;
    //     ss << "this is task1 "<< i <<" with p 2";
        
    //     logger->info(logger->CreateEvent(ss.str()));
    //     }
    // })));
    //     if(i == 5 ){
    //         pool->enqueue(std::move<Task>(Task(1,[](){
    //         std:: cout << "This is task2 with p 5" << std::endl;
    //         })));
    //     }
    // }

    MessageQueue::ptr messagequeue = MessageQueue::ptr(new MessageQueue());
        pool->addLoopThread(
        Thread::ptr(new Thread("Reducer",
            [messagequeue, pool]()
            {
                // Semaphore::ptr producer = messagequeue->getsemproducer();
                // Semaphore::ptr reducer = messagequeue->getsemreduecer();
                LogManager::ptr logger = LogManager::instance();
                //
                
                while (true)
                {

                    cout << "This is showmaker" << endl;
                    // reducer->wait();
                    Message::ptr message = messagequeue->pop();
                    // producer->notify();

                    // std::cout << "this is reducer" << endl;
                    logger->debug(
                        logger->CreateEvent(
                            string((char *)(message->getdata()),message->getsize())));
                }
            })));
    messagequeue->start_on_threadpool(pool, 8000);

    
    while(true);
}