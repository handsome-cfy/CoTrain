#ifndef __CoTrain_SOCKET_H__
#define __CoTrain_SOCKET_H__

#include<string>
#include<memory>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

#include"log.h"


namespace CoTrain{

class Address{
public:
    //总得有个字符串作为地址吧
    typedef std::shared_ptr<Address> ptr;
    std::string m_address;
    
    Address() = default;

    virtual bool operator< (const Address& other) const =0;
    virtual bool operator==(const Address& other) const =0;
    virtual bool operator!=(const Address& other) const =0;

    virtual ~Address() = default;
    
    //依据端口号初始化地址参数
    virtual void init_port(int port) = 0;

    virtual sockaddr_in* getsockaddr() = 0;
    //线程安全
    std::mutex m_mutex;
};

class AddressType{
public:
    enum class Type{
        IPV4 = 1,
        IPV6 = 2,    
    };
};

class IPAddress : public Address{
public:

    typedef std::shared_ptr<IPAddress> ptr;

    virtual std::string toString() const = 0;
    
    virtual std::vector<uint8_t> toByte() const = 0;

    virtual AddressType::Type getType() const = 0;



};

class IPV4Address : public IPAddress{
public:

    typedef std::shared_ptr<IPV4Address> ptr;

    // 构造函数，接受一个字符串地址作为参数，例如 "192.168.0.1"
    explicit IPV4Address(const std::string& addr);

    // 重写基类的纯虚函数
    std::string toString() const override;
    std::vector<uint8_t> toByte() const override;
    AddressType::Type getType() const override;
    bool operator<(const Address& other) const override;
    bool operator==(const Address& other) const override;
    bool operator!=(const Address& other) const override;
    void init_port(int port) override;

    sockaddr_in* getsockaddr() override {return &m_sockaddr;}
private:

    sockaddr_in m_sockaddr;
};

class Socket{
public:
    typedef std::shared_ptr<Socket> ptr;
    virtual bool connect(Address::ptr address,uint32_t port) = 0;
    //是否已连接
    bool isconnect(){return b_connect;}
    //断开连接
    virtual void disconnect() = 0;

    virtual bool send(const void* data, size_t size) = 0;

    virtual bool receive(void* buffer, size_t size) = 0;


protected:
    bool b_connect = false;
    int m_socketid;
    //线程安全
    std::mutex m_mutex;

};

class TcpSocket : public Socket{
public:

    TcpSocket(){};
    bool connect(Address::ptr address, uint32_t port) override;
    bool send(const void* data, size_t size) override;
    bool receive(void* buffer, size_t size) override;
    void disconnect() override;

protected:

private:
    uint32_t m_port;

    Address::ptr m_S_addr;
    Address::ptr m_C_addr;

    //超时时间
    int timeoutInSeconds = 5;


};

}
#endif