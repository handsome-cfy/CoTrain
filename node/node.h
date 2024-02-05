#ifndef __CoTrain_NODE_H__
#define __CoTrain_NODE_H__

#include"../message/messagequeue.h"
#include"../socket.h"

namespace CoTrain{

class Node{
public:
    typedef std::shared_ptr<Node> ptr;
    
    Node(){}
    // virtual Node(Config::ptr config);
    
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
    ClientNode(){}
    ClientNode(ClientNodeConfig::ptr config);

    bool isconnect() override;
    bool connect();
private:
    TcpSocket::ptr m_socket;
};
}
#endif