#include "node.h"

namespace CoTrain{
CoTrain::ServerNode::ServerNode(ServerNodeConfig::ptr config, bool start)
{
    // ServerNodeConfig::ptr serverconfig = static_cast<ServerNodeConfig::ptr> (config);
    m_threadpool = ThreadPool::ptr(new ThreadPool(config));

    m_messagequeue = MessageQueue::ptr(new MessageQueue(config));

    if(start){
        //  依据config文件设置监听的端口
        m_messagequeue->start_on_threadpool(m_threadpool,config->getport());
    }
}
bool ClientNode::isconnect()
{
    return false;
}
bool ClientNode::connect()
{
    return m_socket->connect();
}
void ClientNode::alive()
{
    while(!m_stop){
        // 每过3s向server服务器发送信息表明当前节点可靠
        std::this_thread::sleep_for(std::chrono::seconds(3));

        ComMessage::ptr alivemessage = ComMessage::ptr(new ComMessage(
            ComMessageType::ComType::AlIVE,m_idmananger->generateID()->encode()
        ));
        // std::cout << m_idmananger->generateID()->encode() << std::endl;
        std::cout << std::string((char *)(alivemessage->getdata()), alivemessage->getsize()) << std::endl;

        m_socket->send(alivemessage);

    }
}
}