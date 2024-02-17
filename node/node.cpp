#include "node.h"

namespace CoTrain{
CoTrain::ServerNode::ServerNode(ServerNodeConfig::ptr config, bool start)
{
    // ServerNodeConfig::ptr serverconfig = static_cast<ServerNodeConfig::ptr> (config);
    m_threadpool = ThreadPool::ptr(new ThreadPool(config));

    m_messagequeue = MessageQueue::ptr(new MessageQueue(config));
    m_filequeue = MessageQueue::ptr(new MessageQueue(config));

    if(start){
        //  依据config文件设置监听的端口
        m_messagequeue->start_on_threadpool(m_threadpool,config->getComport());
        m_filequeue->start_on_threadpool(m_threadpool,config->getfileport());
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
void ServerNode::addConnectedNode(uint64_t machineid, ConnectedNode::ptr node)
{
    std::unique_lock<std::mutex> lock(m_nodeset_mutex);
    m_id2node.insert(std::make_pair(machineid,node));
}
void *ServerNode::receiveFile(std::string filepath)
{
    // std::ofstream file(filepath, std::ios::binary);
    // if (file.is_open()) {
    //     // 接收文件大小
    //     uint64_t fileSize;
    //     m_socket->receive(&fileSize, sizeof(fileSize));

    //     // 创建一个缓冲区来接收文件内容
    //     const int bufferSize = 1024; // 缓冲区大小
    //     char buffer[bufferSize];

    //     // 分块接收文件内容
    //     while (fileSize > 0) {
    //         int chunkSize = std::min(fileSize, static_cast<uint64_t>(bufferSize));
    //         m_socket->receive(buffer, chunkSize);
    //         file.write(buffer, chunkSize);
    //         fileSize -= chunkSize;
    //     }

    //     // 关闭文件
    //     file.close();

    //     // return true;
    // }
    // // return false;
    return nullptr;
}
void ServerNode::ProcessMessage(Message::ptr message)
{
    message->decodeHead();
    Task::ptr task = nullptr;
    // bufmessage
    if(message->getType()==0){
        BufMessage::ptr bufM = std::static_pointer_cast<BufMessage>(message);
        bufM->wait();
        std::string s = bufM->toString();
        std::cout<<"that is get" << std::endl;
        std::cout << s << std::endl;
        // LogManager::ptr logger =  LogManager::instance();
        // logger->debug(
        //     logger->CreateEvent(s)
        // );
    }else{//commessage
        ComMessage::ptr comM = ComMessage::ptr(message);
        ComMessageType::ComType type = ComMessageType::ComType(comM->getType());
            switch (type)
            {
            case ComMessageType::ComType::AlIVE:
                /* code */
                task = Task::ptr(new  Task(5,
                    [message,this](){
                        SnowFlakeID id;
                        id.decode(message->getUniqueID());
                        uint64_t workerid = id.getWorkerId();
                        this->alivenode(workerid);
                    }
                ));
                break;
            case ComMessageType::ComType::CONNECT:
            
                task = Task::ptr(new Task(1,
                [message,this](){
                    SnowFlakeID id;
                    id.decode(message->getUniqueID());
                    uint64_t workerid = id.getWorkerId();

                    ConnectedNode::ptr node = ConnectedNode::ptr(new ConnectedNode(workerid));
                    this->addConnectedNode(workerid,node);

                }
                ));
                break;

            case ComMessageType::ComType::FILE:
                m_filequeue->addBufMessageToReceive();
                break;
            default:
                break;
            }

            if(task){
                addTask(task);
            }
    }
}
void ServerNode::proccess()
{
    m_threadpool->addLoopThread(
        Thread::ptr(new Thread("ServerComProccess",
        [this](){
            ProcessMessage(this->m_messagequeue->pop());
        }))
    );
    m_threadpool->addLoopThread(
        Thread::ptr(new Thread("ServerBufProccess",
        [this](){
            std::this_thread::sleep_for(std::chrono::seconds(3));
            ProcessMessage(this->m_filequeue->pop());
        }))
    );
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
void ClientNode::File()
{
    ComMessage::ptr filemessage = ComMessage::ptr(new ComMessage(
        ComMessageType::ComType::FILE,m_idmananger->generateID()->encode()
    ));
    // std::cout << m_idmananger->generateID()->encode() << std::endl;
    // std::cout << std::string((char *)(filemessage->getdata()), filemessage->getsize()) << std::endl;
    std::cout << "here is the file" << std::endl;

    m_socket->send(filemessage);
}
bool ClientNode::sendfile(std::string filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (file.is_open()) {
        this->File();

        // 获取文件大小
        file.seekg(0, std::ios::end);
        uint64_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // 使用一个单独的socket连接将文件内容发送给服务器
        TcpSocket::ptr fileSocket = TcpSocket::ptr(new TcpSocket(m_socket));
        fileSocket->connect(m_fileport);
        fileSocket->send(&fileSize, sizeof(fileSize)); // 发送文件大小

        // 分块发送文件内容
        const int bufferSize = 1024; // 缓冲区大小
        char buffer[bufferSize];
        while (fileSize > 0) {
            int chunkSize = std::min(fileSize, static_cast<uint64_t>(bufferSize));
            file.read(buffer, chunkSize);
            fileSocket->send(buffer, chunkSize);
            fileSize -= chunkSize;
        }

        // 关闭文件
        file.close();

        fileSocket->disconnect();

        return true;
    }
    return false;
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