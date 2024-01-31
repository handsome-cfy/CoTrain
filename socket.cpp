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


bool TcpSocket::connect(Address::ptr c_address, uint32_t port)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if(b_connect&&m_port==port){
        return true;
    }
    m_port = port;
    c_address->init_port(m_port);

    m_socketid = socket(AF_INET,SOCK_STREAM,0);

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

Message::ptr TcpServer::getMessage()
{
    //TODO getMessage
    return Message::ptr();
}

void TcpServer::stop()
{
    //TODO stop
}
}

#endif