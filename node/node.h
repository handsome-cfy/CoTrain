#ifndef __CoTrain_NODE_H__
#define __CoTrain_NODE_H__

#include"../message/messagequeue.h"
#include"../socket.h"
// #include"task.h"

namespace CoTrain{

class Node{
public:
    typedef std::shared_ptr<Node> ptr;
    
    Node(){}
    Node(Config::ptr config){}
    ~Node(){}
    
    //来判断当前这个节点是否连接
    // virtual bool isconnect()=0;

    uint64_t getmachineID(){return m_machineID;}
    void setmachineID(uint64_t var){m_machineID = var;}
private:
    // 统一标识号
    uint64_t m_machineID;
};

// 用来保存当前已经连接到服务端端节点的相关信息
class ConnectedNode : public Node{
public:
    typedef std::shared_ptr<ConnectedNode> ptr;

    ConnectedNode(TcpSocket::ptr socket): m_socket(socket){}
    ConnectedNode(uint64_t workerid): m_workerid(workerid){}

    // 用来实现alive的计数功能
    void AliveCount();

    bool getvalid();
    void setvalid(bool val);

    // 用来增加alive计数的函数（线程安全）
    void alive_count_increase();
private:
    // 用来记录当前alive的剩余次数，当次数减到-1的时候，认为该节点失效
    uint16_t m_alive_count = 0;
    // alive count的互斥锁
    std::mutex m_alive_mutex;

    // 用来记录当前节点是否有效的变量，如果alive失效了，那么设置为false，该节点不可用
    bool b_valid = true;
    // 用来访问b_valid的mutex
    std::mutex m_valid_mutex;

    // 连接到该节点的socket 指针
    TcpSocket::ptr m_socket;

    // alive count 的最大值
    constexpr static uint16_t MAX_ALIVE_COUNT = 16;

    //  workerid
    uint64_t m_workerid;

};

// 服务端的节点
class ServerNode : public Node{
friend class ConnectedNode;

public:
    typedef std::shared_ptr<ServerNode> ptr;

    ServerNode(ServerNodeConfig::ptr config,bool start=false);
    ~ServerNode(){};

    // bool alive();

    // 用来将给定machineid对应的node节点的alive计数增加
    void alivenode(uint64_t machineid);

    // 增加任务到线程池（必须不是循环任务）
    void addTask(Task::ptr val);

    // 增加一个连接上的节点
    void addConnectedNode(uint64_t machineid, ConnectedNode::ptr node);

    // 得到消息队列中的Com消息
    ComMessage::ptr getComMessage(){return m_messagequeue->pop();}

    // 接受分块文件
    void* receiveFile(std::string filepath);

    // 处理消息队列中的信息
    void ProcessMessage(Message::ptr message);

    // loop to proccess
    void proccess();
private:
    // 拥有一个消息队列来处理操作
    MessageQueue::ptr m_messagequeue;
    // 拥有一个消息队列来接受文件
    MessageQueue::ptr m_filequeue;
    // 拥有一个线程池来处理计算任务
    ThreadPool::ptr m_threadpool;

    // 拥有一个map来保存当前已经连接的节点
    std::map<uint64_t,ConnectedNode::ptr> m_id2node;
    // 节点map的锁
    std::mutex m_nodeset_mutex;
    // 用来生成服务器的任务
    // TaskFactory::ptr m_taskfactory;
};

// 客户端的节点，是用来在其他机器上连接服务器节点的
class ClientNode : public Node{
public:
    typedef std::shared_ptr<ClientNode> ptr;
    ClientNode(){}
    ClientNode(ClientNodeConfig::ptr config):Node(config){
    m_socket = TcpSocket::ptr(new TcpSocket(config));

    m_machineID = config->getMachineID();
    m_idmananger = UniqueIDMananger::ptr(new UniqueIDMananger(UniqueIDMananger::IDtype::SnowFlake,m_machineID));
    
    m_fileport = config->getFilePort();
    }

    // bool  isconnect() override;
    bool connect();

    void alive();

    //发送comtype为file的message
    void File();
    bool sendfile(std::string filepath);


private:
    bool m_stop = false;

    std::mutex m_socket_mutex;
    TcpSocket::ptr m_socket;
    // 控制生成id
    UniqueIDMananger::ptr m_idmananger;
    // 机器的名称
    uint64_t m_machineID;

    // 文件服务器的端口
    uint32_t m_fileport=8001;

};
}
#endif