#ifndef __CoTrain_MESSAGE_H__
#define __CoTrain_MESSAGE_H__

#include<memory>
#include<string>
#include"../log.h"
#include"uniqueid.h"

namespace CoTrain{


class Message{
public:
    typedef std::shared_ptr<Message> ptr;

    void* getdata(){return m_data;}
    void setdata(void* val){m_data = val;}

    void setID(UniqueID::ptr val){m_uniqueid = val;}
    UniqueID::ptr getID(){return m_uniqueid;}

    Message(){}
    //对于不同的message，使用不同的m——data释放策略
    virtual ~Message() = 0;
    virtual void* getdata_withid() = 0;

protected:
    void * m_data;
    UniqueID::ptr m_uniqueid;
};
//  带有一个缓冲区的message，设计用来传递数据
class BufMessage: public Message{

};

//  用于构建分布式系统的message
class ComMessage: public Message{

};
}
#endif