#ifndef __CoTrain_SOCKET_H__
#define __CoTrain_SOCKET_H__

#include<string>
#include<memory>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

#include"message/message.h"

#include"log.h"


namespace CoTrain{

class BufMessage;

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

    virtual uint16_t getsockaddr_size() = 0;
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

    std::string getip(){return m_ip;}
    void setip(std::string val){m_ip = val;}
    uint32_t getport(){return m_port;}
    void setport(uint32_t val){m_port =val;}
private:
    std::string m_ip;
    uint32_t m_port;

};

class IPV4Address : public IPAddress{
public:

    typedef std::shared_ptr<IPV4Address> ptr;

    // 构造函数，接受一个字符串地址作为参数，例如 "192.168.0.1"
    explicit IPV4Address(const std::string& addr);
    explicit IPV4Address(){}

    // 重写基类的纯虚函数
    std::string toString() const override;
    std::vector<uint8_t> toByte() const override;
    AddressType::Type getType() const override;

    //TODO: fix the operator by compare ip and port
    bool operator<(const Address& other) const override;
    bool operator==(const Address& other) const override;
    bool operator!=(const Address& other) const override;
    void init_port(int port) override;

    sockaddr_in* getsockaddr() override {return &m_sockaddr;}
    uint16_t getsockaddr_size() override {return sizeof(m_sockaddr);}
private:

    sockaddr_in m_sockaddr;
};

class Socket{
public:
    typedef std::shared_ptr<Socket> ptr;
    virtual bool connect(Address::ptr address,uint32_t port) = 0;
    virtual bool connect() = 0;
    //是否已连接
    bool isconnect(){return b_connect;}
    int getsocketid(){return m_socketid;}
    //断开连接
    virtual void disconnect() = 0;

    virtual bool send(const void* data, size_t size) = 0;
    virtual bool send(const Message::ptr message) = 0;

    virtual bool receive(void* buffer, size_t size) = 0;
    virtual Message::ptr receive() = 0;

    virtual bool bind(const Address::ptr address) = 0;

    virtual int accept(Address::ptr address) = 0;

protected:
    bool b_connect = false;
    int m_socketid;
    //线程安全
    std::mutex m_mutex;

};

class TcpSocket : public Socket{
public:

    TcpSocket();
    TcpSocket(int socketid, Address::ptr address);
    TcpSocket(ClientNodeConfig::ptr config);

    //用于连接其他设备
    bool connect(Address::ptr address, uint32_t port) override;
    bool connect() override;
    bool send(const void* data, size_t size) override;
    bool send(const Message::ptr message) override;
    bool receive(void* buffer, size_t size) override;
    Message::ptr receive() override;
    void disconnect() override;
    bool bind(const Address::ptr address) override;

    virtual int accept(Address::ptr address){}

protected:
    Address::ptr m_S_addr;

private:
    // 判断地址是否是有效的
    bool is_address_valid = false;
    uint32_t m_port;
    Address::ptr m_C_addr;

    //超时时间
    int timeoutInSeconds = 5;


};

//用来处理当，接入一个新的设备的时候，分配一个新的端口
class TcpListenSocket: public TcpSocket{
public:
    TcpListenSocket(){};
    TcpListenSocket(Address::ptr server_address, uint16_t port);
    int accept(Address::ptr address) override;
private:
    static const uint16_t retry_max_time = 10;

};

class TcpServer{
public:
    typedef std::shared_ptr<TcpServer> ptr;
    //通过这个方法从网络中获取message
    Message::ptr getMessage();
    void stop();
    TcpServer();
    TcpServer(ServerNodeConfig::ptr config);
    // 用于一直监听某个端口
    void Listen(uint32_t port);
    void Listen(ThreadPool::ptr pool,uint32_t port);

    // 初始化服务器阐述
    void Init(uint32_t port);

    // 对于获取到的新连接信息，把它连接上并且加入到list之中，分配一个新的port
private:
    //建立的连接的vector
    std::vector<Socket::ptr> m_socket_list;
    bool m_stop = false;

    //服务器本地的地址
    IPV4Address::ptr m_server_address;
    //从哪个端口开始分配
    uint16_t m_base_port = 11000;
    //最大的连接数量
    uint16_t m_max_connect = 100;

    TcpListenSocket::ptr m_listen_socket;

    LogManager::ptr logger;

    std::string m_threadname="TcpServer";

    uint32_t get_new_port();

    // 当前访问的socket位置
    uint16_t m_pos = 0;
};

}
#endif