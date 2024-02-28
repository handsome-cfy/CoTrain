#ifndef __CoTrain_SOCKET_CPP__
#define __CoTrain_SOCKET_CPP__
#include "socket.h"

namespace CoTrain{
CoTrain::IPV4Address::IPV4Address(const std::string &addr)
{
    m_address = addr;
}

std::string CoTrain::IPV4Address::toString() const
{
    return m_address;
}

std::vector<uint8_t> CoTrain::IPV4Address::toByte() const
{
    return std::vector<uint8_t>();
}

AddressType::Type CoTrain::IPV4Address::getType() const
{
    return AddressType::Type::IPV4;
}

bool CoTrain::IPV4Address::operator<(const Address &other) const
{
    return false;
}

bool CoTrain::IPV4Address::operator==(const Address &other) const
{
    return false;
}
bool IPV4Address::operator!=(const Address &other) const
{
    return false;
}
void IPV4Address::init_port(int port)
{
    bzero(&m_sockaddr, sizeof(m_sockaddr));
    m_sockaddr.sin_family = AF_INET;
    m_sockaddr.sin_port = htons(port);
    m_sockaddr.sin_addr.s_addr = inet_addr(toString().c_str());

}

TcpSocket::TcpSocket()
{
    m_socketid = socket(AF_INET,SOCK_STREAM,0);
}

TcpSocket::TcpSocket(int socketid, Address::ptr address)
{
    m_socketid = socketid;
    m_C_addr = address;
    b_connect = true;
    is_address_valid = true;
}

TcpSocket::TcpSocket(ClientNodeConfig::ptr config)
{
    m_socketid = socket(AF_INET,SOCK_STREAM,0);
    std::string ip  = config->getServerIP();
    uint32_t port = config->getServerPort();
    m_C_addr = IPV4Address::ptr(new IPV4Address(ip));
    m_port = port;

    //设置地址与端口合法
    is_address_valid = true;
}

TcpSocket::TcpSocket(const TcpSocket::ptr other){
    // 复制基本类型成员变量
    b_connect = other->b_connect;
    timeoutInSeconds = other->timeoutInSeconds;

    // 复制指针类型成员变量
    m_S_addr = other->m_S_addr;
    m_C_addr = other->m_C_addr;

    // 创建一个新的socketid
    m_socketid = socket(AF_INET, SOCK_STREAM, 0);

    is_address_valid = other->is_address_valid;
}

TcpSocket::ptr TcpSocket::operator=(const TcpSocket::ptr other)
{
    return TcpSocket::ptr(new TcpSocket(other));
}

bool TcpSocket::connect()
{
    if(is_address_valid){
        return this->connect(m_C_addr,m_port);
    }
    return false;
}

bool TcpSocket::connect(Address::ptr c_address, uint32_t port)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if(b_connect&&m_port==port){
        return true;
    }
    m_port = port;
    c_address->init_port(m_port);

    

    //设置超时时间

    m_C_addr = c_address;
    // m_C_addr->init_port(m_port);

    int ret = ::connect(m_socketid, (sockaddr *)m_C_addr->getsockaddr(), sizeof(*(m_C_addr->getsockaddr())));
    
    if(ret == -1){
        b_connect = false;

        LogManager::ptr log =  LogManager::instance();
        log->debug(log->CreateEvent("unConnected"));
    }else{
        b_connect = true;

        LogManager::ptr log =  LogManager::instance();
        log->debug(log->CreateEvent("Connected"));
    }

    return b_connect;
}

bool TcpSocket::send(const void *data, size_t size)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if(b_connect&&m_socketid!=-1){
        int sended = ::send(m_socketid,data,size,0);
        if(sended == -1){
            return false;
        }
        return true;
    }else{
        return false;
    }
}

bool TcpSocket::connect(uint32_t port)
{
    if(is_address_valid){
        return this->connect(m_C_addr,port);
    }
    return false;
}

bool TcpSocket::send(const Message::ptr message)
{
    //TODO: 完成从信息到传输数据的编码

    return send(message->getdata(),message->getsize());
    // return false;
}
bool TcpSocket::send(const BufMessage::ptr message)
{
    // fileSocket->connect(m_fileport);
    uint64_t fileSize = 0;
    fileSize = message->getsize();
    send(&fileSize, sizeof(fileSize)); // 发送文件大小

    // 分块发送文件内容
    const int bufferSize = 1024; // 缓冲区大小
    char *buffer = nullptr;
    buffer = (char*)message->getdata();
    if(buffer==nullptr){
        return false;
    }
    while (fileSize > 0) {
        int chunkSize = std::min(fileSize, static_cast<uint64_t>(bufferSize));
        send(buffer, chunkSize);
        fileSize -= chunkSize;
        buffer = buffer + chunkSize;
    }
    return true;
}
bool TcpSocket::receive(void *buffer, size_t size)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (b_connect && m_socketid != -1) {
        // // 设置超时时间为5秒
        // struct timeval timeout;
        // timeout.tv_sec = 5;
        // timeout.tv_usec = 0;
        // if (setsockopt(m_socketid, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        //     perror("setsockopt failed");
        //     // 错误处理
        //     return false;
        // }

        int receivedBytes = recv(m_socketid, buffer, size, 0);
        if (receivedBytes == -1) {
            // 接收失败的处理
            int errorCode = errno;
            if (errorCode == EAGAIN || errorCode == EWOULDBLOCK) {
                std::cerr << "No data available to receive. Retry later." << std::endl;
                // 重新尝试接收或使用其他处理方法
            } else {
                std::cerr << "recv failed with error code: " << errorCode << std::endl;
                perror("recv failed");
                // 处理其他错误
            }
            return false;
        }else if (receivedBytes == 0){
            b_connect = false;
        }
        // 接收成功的处理
        return true;
    }
    else {
        return false;
    }
}

Message::ptr TcpSocket::receive(bool isFixedSize)
{  
    if(isFixedSize){
        Message::ptr message = Message::ptr(new ComMessage());
        uint16_t size = message->getsize();
        if(receive(message->getdata(),size)){
            return message;
        }
    }else{

        // uint64_t size = 0;
        // if (!receive((&size), sizeof(size))) {
        //     return nullptr;
        // }
        // 定义固定大小的字符串长度
        const int fixedSize = 10;

        // 创建一个固定大小的缓冲区用于接收字符串
        char buffer[fixedSize + 1]; // 加1是为了留出空间存储字符串结尾的 '\0'

        // 使用接收端的 socket 进行接收
        int bytesReceived = receive(buffer, fixedSize);

        std::string receivedStr = "";
        // 确保接收到了固定大小的数据
        if (false) {
            // 处理接收数据不完整的情况
            // 可以根据实际需求进行错误处理或重试逻辑
        } else {
            // 接收到了固定大小的数据
            buffer[fixedSize] = '\0'; // 添加字符串结尾的 '\0' 字符

            // 将接收到的字符串转换为 std::string
            receivedStr = buffer;
            
            send(buffer,fixedSize);
            // 在这里对接收到的字符串进行处理
            // 可以根据实际需求进行相应的操作
            // 例如输出到控制台或进行其他业务逻辑处理
        }
        uint64_t size = 0;
        try {
            size = std::stoull(receivedStr);
            // 在这里处理接收到的 uint64_t 类型的数据
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument error: " << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Out of range error: " << e.what() << std::endl;
        }
        // std::vector<char> rec(fileSize);
        // uint64_t receivedSize = 0;
        // while (receivedSize < fileSize) {
        //     uint64_t remain = fileSize - receivedSize;
        //     uint64_t chunkSize = std::min(remain, static_cast<uint64_t>(MAX_CHUNK_SIZE));
        //     if (!receive(rec.data() + receivedSize, chunkSize)) {
        //         return nullptr;
        //     }
        //     receivedSize += chunkSize;
        // }

        BufMessage::ptr message = BufMessage::ptr(new BufMessage(size));
        // 根据大小分块接收消息内容
        uint16_t receivedSize = 0;
        while (receivedSize < size) {
            uint16_t remainingSize = size - receivedSize;
            uint16_t chunkSize = std::min(remainingSize, static_cast<uint16_t>(MAX_CHUNK_SIZE));

            if (!receive((char *)(message->getdata()) + receivedSize, chunkSize)) {
                return nullptr;
            }

            receivedSize += chunkSize;
        }

        message->setsize(size);

        // TODO delete
        std::string s = std::string((char *)(message->getdata()),size);

        std::cout << s <<std::endl;

        std::cout << message->toString() << std::endl;

        message->notify();
        
        return message;
    }

    return nullptr;
}

void TcpSocket::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if(b_connect&&m_socketid!=-1){
        ::close(m_socketid);
        b_connect = false;
    }
}

bool TcpSocket::bind(const Address::ptr address)
{
    int ret = ::bind(m_socketid,(sockaddr *)(address->getsockaddr()), address->getsockaddr_size());
    if(ret != -1){
        return true;
    }
    return false;
}

Message::ptr TcpServer::getMessage(bool isBuf)
{
    if(m_socket_list.size()>0){
        auto socket = m_socket_list[m_pos];
        m_pos = (m_pos + 1)%(m_socket_list.size());
        if(socket->isconnect()){
            return socket->receive(!isBuf);
        }else{
            m_pos = m_pos - 1;
            auto it = std::find(m_socket_list.begin(), m_socket_list.end(), socket);
            if (it != m_socket_list.end()) {
                m_socket_list.erase(it);
            }
        }
    }
    return Message::ptr();
}

void TcpServer::stop()
{
    m_stop = true;
    for(auto socket : m_socket_list){
        socket->disconnect();
    }

}
bool TcpServer::sendMessage(Socket::ptr socket,Message::ptr message,bool isBuf)
{
    if(socket->isconnect()){
        socket->send(message);
    }
    return false;
}
TcpServer::TcpServer()
{
    m_listen_socket = TcpListenSocket::ptr(new TcpListenSocket());
    m_server_address = IPV4Address::ptr(new IPV4Address());
}
TcpServer::TcpServer(ServerNodeConfig::ptr config)
{
    m_listen_socket = TcpListenSocket::ptr(new TcpListenSocket());
    m_server_address = IPV4Address::ptr(new IPV4Address());
}
void TcpServer::Listen(uint32_t port)
{
    Init(port);
    int ret = ::listen(m_listen_socket->getsocketid(),m_max_connect);
    if(ret == -1){
        logger->debug(logger->CreateEvent(
            "Listen Fialed",m_threadname
        ));
        return;
    }

    while(!m_stop){
        if(m_socket_list.size() >= m_max_connect){
            
            //如果连接数量达到上限，停止listen
            continue;
        }
        IPV4Address::ptr client_address = IPV4Address::ptr(new IPV4Address());
        // m_listen_socket = static_cast<TcpListenSocket::ptr>(m_listen_socket);
        // cfd 就是连接的客户端的套接字
        int cfd = m_listen_socket->accept(client_address);
        if (cfd < 0){
            // no accepts
            continue;
        }else{
            char ip[16] = "";
            u_int32_t port = 0;
            inet_ntop(AF_INET, &(client_address->getsockaddr()->sin_addr.s_addr),ip,16);
            client_address->setip(std::string(ip));
            port = ntohs(client_address->getsockaddr()->sin_port);
            //TODO Maybe here could create a address(addr_);
            client_address->setport(port);

            TcpSocket::ptr client_socket = TcpSocket::ptr(new TcpSocket(cfd,client_address));
            m_socket_list.push_back(client_socket);

            std::stringstream ss;
            ss << "The IP: " << ip << " Port: " << port << " connected" << std::endl;

            {
                logger->info(
                    logger->CreateEvent(
                        ss.str(),m_threadname
                    )
                );
            }
            
        }
    }

}
void TcpServer::Listen(ThreadPool::ptr pool, uint32_t port)
{
    if(pool != nullptr){
        pool->addLoopThread(
            Thread::ptr(new Thread(
                m_threadname,
                [this,port](){
                    this->TcpServer::Listen(port);
                }
            )
        ));
    }
}
void TcpServer::Init(uint32_t port)
{
    m_server_address->init_port(port);
    m_server_address->getsockaddr()->sin_addr.s_addr = htonl(INADDR_ANY);

    if(m_listen_socket == nullptr){
        m_listen_socket = TcpSocket::ptr(new TcpSocket());
    }
    logger = LogManager::instance();

    if(m_listen_socket->bind(m_server_address)){
        {
            logger->debug(logger->CreateEvent(
                "Listen Bind Success",m_threadname
            ));
        }

    }else{
        //bind fail
        logger->error(logger->CreateEvent(
            "Listen Bind Failed",m_threadname
        ));
    }

}
uint32_t TcpServer::get_new_port()
{
    //TODO 这玩意不可靠
    return m_base_port + m_socket_list.size();
}
TcpListenSocket::TcpListenSocket(Address::ptr server_address, uint16_t port)
{
    m_S_addr = server_address;

    uint16_t retry = retry_max_time;
    while(retry--){
        if(this->connect(m_S_addr,port)){
            break;
        }
    }

}

int TcpListenSocket::accept(Address::ptr address)
{
    socklen_t cil_len = static_cast<socklen_t>(address->getsockaddr_size());
    return ::accept(m_socketid,(sockaddr*)address->getsockaddr(),&cil_len);
}
}

#endif