#include"../log.h"
#include"../thread.h"
#include"../socket.h"
#include"../message/uniqueid.h"
#include"../message/message.h"

#include"nlohmann/json.hpp"

#include<utility>
#include<memory>

// #define __CoTrain_LOG_H__
using namespace CoTrain;

int main(void){
    //初始化日志系统
    LogManager::ptr log = LogManager::instance();
    // log->init();
    
    //初始化线程池
    // ThreadPool::ptr pool = ThreadPool::ptr(new ThreadPool());
    // pool->init();
    // log->info(log->CreateEvent("ThreadInited"));

    Socket::ptr socket = Socket::ptr(new TcpSocket());
    socket->connect(Address::ptr(new IPV4Address("127.0.0.1")) ,8000);
    Message::ptr message =  socket->receive();
    std::cout << message->getsize() << std::endl;
    for(uint16_t i = 0; i < message->getsize(); i++){
        std::cout << *(static_cast<char*>((char*)message->getdata()+i));
    }
    std::cout << std::endl;
    
    // MessageQueue::ptr message_queue = MessageQueue::ptr()

    
    // for(int i = 0; i < 30; i++){
    //     pool->enqueue(std::move<Task>(Task(5,[i,log](){
    //     {
    //     // std::unique_lock<std::mutex>lock(log_mutex) ;
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     std::stringstream ss;
    //     ss << "this is task1 "<< i <<" with p 2";
        
    //     log->info(log->CreateEvent(ss.str()));
    //     }
    // })));
    //     if(i == 5 ){
    //         pool->enqueue(std::move<Task>(Task(1,[](){
    //         std:: cout << "This is task2 with p 5" << std::endl;
    //         })));
    //     }
    // }

    return 0;
}