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

    void* getdata(){return m_data;}
    void setdata(void* val){m_data = val;}

    void setID(UniqueID::ptr val){m_uniqueid = val;}
    UniqueID::ptr getID(){return m_uniqueid;}

    uint16_t getmaxsize(){return max_buf_len;}
    uint16_t getsize(){return m_size;}
    void setsize(uint16_t val){m_size = val;}

    Message(){}
    //对于不同的message，使用不同的m——data释放策略
    // virtual ~Message() = 0;
    // virtual void* getdata_withid() = 0;
    virtual ~Message(){
    }

protected:
    void * m_data;
    Bufptr m_buf;
    UniqueID::ptr m_uniqueid;
    constexpr static uint16_t max_buf_len = 1500;
    uint16_t m_size;

};
//  带有一个缓冲区的message，设计用来传递数据
class BufMessage: public Message{
public:
    void showdata();
    BufMessage();
    ~BufMessage();

};

class ComMessageType{
public:
    typedef std::shared_ptr<ComMessageType> ptr;
    enum ComType{
        CONNECT,
        CLOSE,
    };
};

//  用于构建分布式系统的message
class ComMessage: public Message{
public:
    ComMessage(uint32_t port){
        m_messagetype = ComMessageType::ComType::CONNECT;
        // char * buf = static_cast<char *>(m_data);
        char buf[max_com_len] = "";
        buf[0] = int(m_messagetype);
        std::string s_port = std::to_string(port);
        strcpy(buf+1,s_port.c_str());
        m_data = static_cast<void *>(buf);

    }
    ComMessage();
    ~ComMessage();
private:
    ComMessageType::ComType m_messagetype;
    constexpr static uint16_t max_com_len = 150;
};


}
#endif