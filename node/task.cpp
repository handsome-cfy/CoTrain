#include "task.h"
namespace CoTrain{
    void TaskFactory::MakeTask(Message::ptr message, ServerNode::ptr servernode)
    {
        if(message->getType()==0){

        }else{
            Task::ptr task = nullptr;
            
            ComMessageType::ComType type = ComMessageType::ComType(message->getType());
            switch (type)
            {
            case ComMessageType::ComType::AlIVE:
                /* code */
                task = Task::ptr(new  Task(5,
                    [message,servernode](){
                        SnowFlakeID id;
                        id.decode(message->getUniqueID());
                        uint64_t workerid = id.getWorkerId();
                        servernode->alivenode(workerid);
                    }
                ));
                break;
            case ComMessageType::ComType::CONNECT:
            
                task = Task::ptr(new Task(1,
                [message,servernode](){
                    SnowFlakeID id;
                    id.decode(message->getUniqueID());
                    uint64_t workerid = id.getWorkerId();

                    ConnectedNode::ptr node = ConnectedNode::ptr(new ConnectedNode(workerid));
                    servernode->addConnectedNode(workerid,node);

                }
                ));
                break;
            default:
                break;
            }

            if(task){
                servernode->addTask(task);
            }
        }
       
    }
    // void TaskFactory::MakeTask(ComMessage::ptr message, ServerNode::ptr servernode, TcpSocket::ptr socket)
    // {
    //     Task::ptr task = nullptr;
    //     ComMessageType::ComType type = ComMessageType::ComType(message->getType());
    //     switch (type)
    //     {

    //     case ComMessageType::ComType::CLOSE:

    //         break;

    //     default:
    //         break;
    //     }

    //     if(task){
    //         servernode->addTask(task);
    //     }
    // }
    void TaskFactory::ProcessMessage(ServerNode::ptr servernode)
    {
        Thread::ptr thread = Thread::ptr(new Thread(std::string("TaskFactory"),
            [this,servernode](){
                LogManager::ptr logger = LogManager::instance();

                while(getstop()){
                    
                                       // reducer->wait();
                    ComMessage::ptr message = servernode->getComMessage();
                    // producer->notify();

                    // std::cout << "this is reducer" << endl;
                    logger->debug(
                        logger->CreateEvent(
                            std::string((char *)(message->getdata()),message->getsize())));

                    message->decodeHead();

                    logger->debug(
                        logger->CreateEvent(
                            message->showheader()
                        )
                    );
                }
            }
        ));
    }
    bool TaskFactory::getstop()
    {
        std::unique_lock<std::mutex> lock(m_stop_mutex);
        return m_stop;
    }
    void TaskFactory::setstop(bool val)
    {
        std::unique_lock<std::mutex> lock(m_stop_mutex);
        m_stop = val;
    }
}
