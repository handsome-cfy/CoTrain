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

        LogMannager::ptr log =  LogMannager::instance();
        log->debug(log->CreateEvent("unConnected"));
    }else{
        b_connect = true;

        LogMannager::ptr log =  LogMannager::instance();
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

bool TcpSocket::send(const Message::ptr message)
{
    //TODO: 完成从信息到传输数据的编码
    return false;
}

bool TcpSocket::receive(void *buffer, size_t size)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (b_connect && m_socketid != -1) {
            int receivedBytes = recv(m_socketid, buffer, size, 0);
            if (receivedBytes == -1) {
                //TODO 接收失败的处理
                return false;
            }
            //TODO 接收成功的处理
            return true;
        }
        else {
            return false;
        }
}

Message::ptr TcpSocket::receive()
{  
    
    Message::ptr message = Message::ptr(new BufMessage());
    uint16_t size = message->getsize();
    if(receive(message->getdata(),size)){
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

Message::ptr TcpServer::getMessage()
{
    //TODO getMessage
    if(m_socket_list.size()>0){
        auto socket = m_socket_list[m_pos];
        m_pos = (m_pos + 1)%(m_socket_list.size());
        if(socket->isconnect()){
            return socket->receive();
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
TcpServer::TcpServer()
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
    logger = LogMannager::instance();

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