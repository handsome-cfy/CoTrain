#ifndef __CoTrain_NODE_H__
#define __CoTrain_NODE_H__

#include"../message/messagequeue.h"
#include"../socket.h"

namespace CoTrain{

class Node{
public:
    typedef std::shared_ptr<Node> ptr;
    
    Node(){}
    Node(Config::ptr config){}
    ~Node(){}
    
    //来判断当前这个节点是否连接
    virtual bool isconnect()=0;
private:

};

// 服务端的节点
class ServerNode : public Node{
public:
    typedef std::shared_ptr<ServerNode> ptr;

    ServerNode(ServerNodeConfig::ptr config,bool start=false);
    ~ServerNode(){};

    // bool alive();
private:
    // 拥有一个消息队列来处理操作
    MessageQueue::ptr m_messagequeue;
    // 拥有一个线程池来处理计算任务
    ThreadPool::ptr m_threadpool;
    // 拥有一个vector来保存当前已经连接的节点
    std::vector<Node::ptr> m_node_list;

};

class ClientNode : public Node{
public:
    typedef std::shared_ptr<ClientNode> ptr;
    ClientNode(){}
    ClientNode(ClientNodeConfig::ptr config):Node(config){
    m_socket = TcpSocket::ptr(new TcpSocket(config));

    m_machineID = config->getMachineID();
    m_idmananger = UniqueIDMananger::ptr(new UniqueIDMananger(UniqueIDMananger::IDtype::SnowFlake,m_machineID));
    }

    bool  isconnect() override;
    bool connect();

    void alive();


private:
    bool m_stop = false;

    TcpSocket::ptr m_socket;
    // 控制生成id
    UniqueIDMananger::ptr m_idmananger;
    // 机器的名称
    uint64_t m_machineID;


};
}
#endif