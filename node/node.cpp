#include "node.h"

namespace CoTrain{
// std::string getNewestFilePath(const std::string& folderPath) {
//     std::string newestFilePath;
//     std::time_t newestFileTime = 0;

//     for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
//         if (entry.is_regular_file()) {
//             std::time_t fileTime = std::chrono::system_clock::to_time_t(std::filesystem::last_write_time(entry));
//             if (fileTime > newestFileTime) {
//                 newestFileTime = fileTime;
//                 newestFilePath = entry.path().string();
//             }
//         }
//     }

//     return newestFilePath;
// }

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
    if(message == nullptr){
        return;
    }
    message->decodeHead();
    Task::ptr task = nullptr;
    // bufmessage
    if(message->getType()==0){
        BufMessage::ptr bufM = std::static_pointer_cast<BufMessage>(message);
        if(bufM == nullptr){
            return;
        }
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
            std::cout << this->m_filequeue->getsize() << std::endl;
            uint16_t size = this->m_filequeue->getsize();
            ProcessMessage(this->m_filequeue->pop());
        }))
    );
}
bool ClientNode::connect()
{
    bool socketconn = true;
    socketconn =m_socket->connect();
    if(!socketconn){
        return false;
    }
    // ComMessage::ptr conmessage = ComMessage::ptr(new ComMessage(
    //     ComMessageType::ComType::CONNECT,m_idmananger->generateID()->encode()
    // ));
    // m_socket->send(conmessage);
    return socketconn;
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

        // 定义固定大小的字符串长度
        const int fixedSize = 10;

        // 获取文件大小
        file.seekg(0, std::ios::end);
        uint64_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // 将文件大小转换为字符串
        std::string fileSizeStr = std::to_string(fileSize);

        // 填充字符串到固定大小
        if (fileSizeStr.size() < fixedSize) {
            std::string padding(fixedSize - fileSizeStr.size(), '0');
            fileSizeStr = padding + fileSizeStr;
        } else if (fileSizeStr.size() > fixedSize) {
            // 处理文件大小超过固定大小的情况
            // 可以根据实际需求进行错误处理或截断逻辑
        }

        // 使用一个单独的 socket 连接将文件大小发送给服务器
        TcpSocket::ptr fileSocket = TcpSocket::ptr(new TcpSocket(m_socket));
        fileSocket->connect(m_fileport);
        fileSocket->send(fileSizeStr.c_str(), fileSizeStr.size()); // 发送文件大小字符串
        
        char sizebuffer[fixedSize] = "";
        fileSocket->receive(sizebuffer, fixedSize);

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
bool ClientNode::startScript()
{
    Thread a = Thread("python_train",
        [this](){
            std::string cmd = "python ";
            cmd += pythonscript_path;
            std::system(cmd.c_str());
        }
    );
    a.join();
    return true;
}
void ClientNode::local_train()
{
    startScript();
    while(!m_stop){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // TODO to determined which file to update


    }
}
std::string ClientNode::gennerateFileName()
{
    // 获取当前时间
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);

    // 构造文件名
    std::stringstream fileNameStream;
    fileNameStream << "file_" << localTime->tm_year + 1900 << "-"
                   << std::setfill('0') << std::setw(2) << localTime->tm_mon + 1 << "-"
                   << std::setfill('0') << std::setw(2) << localTime->tm_mday << "_"
                   << std::setfill('0') << std::setw(2) << localTime->tm_hour << "-"
                   << std::setfill('0') << std::setw(2) << localTime->tm_min << "-"
                   << std::setfill('0') << std::setw(2) << localTime->tm_sec
                   << ".json";

    return fileNameStream.str();
}
// std::vector<std::string> ClientNode::getAllFileNames(const std::string &folderpath)
// {
//     std::vector<std::string> fileNames;

//     for (const auto& entry : std::filesystem::directory_iterator(folderpath)) {
//         if (entry.is_regular_file()) {
//             fileNames.push_back(entry.path().filename().string());
//         }
//     }

//     return fileNames;
// }
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