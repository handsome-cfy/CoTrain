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
ClientNode::ClientNode(ClientNodeConfig::ptr config)
{
    m_socket = TcpSocket::ptr(new TcpSocket(config));
}
bool ClientNode::connect()
{
    return m_socket->connect();
}
}