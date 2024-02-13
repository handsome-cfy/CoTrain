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
void ServerNode::alivenode(uint64_t machineid)
{
    ConnectedNode::ptr node = m_id2node[machineid];
    node->alive_count_increase();
}
void ServerNode::addTask(Task::ptr val)
{
    m_threadpool->enqueue(*val);
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
void ConnectedNode::AliveCount()
{
    uint16_t count = 1;
    while(count > -1){
        {
            std::unique_lock<std::mutex> lock(m_alive_mutex);
            m_alive_count--;
            count = m_alive_count;
        }
        // 每3秒检查一次
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    // alive失效了
    setvalid(false);
}
bool ConnectedNode::getvalid()
{
    std::unique_lock<std::mutex> lock(m_valid_mutex);
    return b_valid;
}
void ConnectedNode::setvalid(bool val)
{
    std::unique_lock<std::mutex> lock(m_valid_mutex);
    b_valid = val;
}
void ConnectedNode::alive_count_increase()
{
    std::unique_lock<std::mutex> lock(m_alive_mutex);
    m_alive_count++;
}
}