#ifndef __CoTrain_MESSAGE_H__
#define __CoTrain_MESSAGE_H__

#include<memory>
#include<string>
#include"../log.h"
#include"../thread.h"

#include"uniqueid.h"

namespace CoTrain{

class BufDelete{
public:
    void operator()(void* ptr){
        std::free(ptr);
    }
};

class Message{
public:
    typedef std::shared_ptr<Message> ptr;
    typedef std::unique_ptr<void,BufDelete> Bufptr;

    std::string showheader();

    void* getdata(){return m_data;}
    void setdata(void* val){m_data = val;}

    uint16_t getmaxsize(){return max_buf_len;}
    uint16_t getsize(){return m_size;}
    void setsize(uint16_t val){m_size = val;}

    Message(){}
    //对于不同的message，使用不同的m——data释放策略
    // virtual ~Message() = 0;
    // virtual void* getdata_withid() = 0;
    virtual ~Message(){
    }

    /*
    对于每一个message，都希望有相同的头部内容
    1-3位：用来表明message的类型
    4-8位：用来表明message的操作
    8-16位：用来表明message的大小

    独特id编码，获取是哪台机器的message
    */

   // 获取和设置消息类型
    uint16_t getType() const { return m_type; }
    void setType(uint16_t type) { m_type = type; }

    // 获取和设置消息操作
    uint16_t getOperation() const { return m_operation; }
    void setOperation(uint16_t operation) { m_operation = operation; }

    // 获取和设置消息大小
    uint16_t getSize() const { return m_size; }
    void setSize(uint16_t size) { m_size = size; }

    // 获取和设置独特ID编码
    uint64_t getUniqueID() const { return m_uniqueID; }
    void setUniqueID(uint64_t uniqueID) { m_uniqueID = uniqueID; }

    void encodeHead();
    void decodeHead();

    void addMessageEnd();

protected:
    // 如果这个值为0，则为buf消息
    uint16_t m_type;      // 消息类型
    uint16_t m_operation; // 消息操作
    uint16_t m_size;      // 消息大小
    uint64_t m_uniqueID;  // 独特ID编码

    std::mutex m_mutex;

    void * m_data;
    Bufptr m_buf;

    constexpr static uint16_t max_buf_len = 1500;
    // uint16_t m_size;

};
//  带有一个缓冲区的message，设计用来传递数据
class BufMessage: public Message{
public:
    typedef std::shared_ptr<BufMessage> ptr;

    void showdata();
    BufMessage();
    BufMessage(uint64_t size);
    ~BufMessage();

    void toFile(std::string filepath);
    std::string toString();

    void wait();
    void notify();
protected:
    Semaphore::ptr m_semaphore;
};

class ComMessageType{
public:
    typedef std::shared_ptr<ComMessageType> ptr;
    enum ComType{
        CONNECT=1,
        AlIVE,
        CLOSE,
        // 用来告诉要等待一个bufmessage
        FILE,
    };
};

//  用于构建分布式系统的message
class ComMessage: public Message{
public:
    ComMessage(ComMessageType::ComType type,uint32_t port){
        m_messagetype = ComMessageType::ComType::CONNECT;
        // char * buf = static_cast<char *>(m_data);
        char buf[max_com_len] = "";
        buf[0] = int(m_messagetype);
        std::string s_port = std::to_string(port);
        strcpy(buf+1,s_port.c_str());
        m_data = static_cast<void *>(buf);
    }

    ComMessageType::ComType getComtype(){return m_messagetype;}
    
    ComMessage(ComMessageType::ComType type, uint64_t uniqueID);
    ComMessage(ComMessageType::ComType type, uint64_t uniqueID, uint64_t bufsize);
    ComMessage();
    ~ComMessage();

    uint64_t getFileSize() const {
        if (m_messagetype == ComMessageType::ComType::FILE) {
            return m_filesize;
        } else {
            return 0;
        }
    }

private:
    ComMessageType::ComType m_messagetype;
    constexpr static uint16_t max_com_len = 50;
    
    // 当type是File的时候有效，表明传递的信息
    uint64_t m_filesize = 0;
};


}
#endif