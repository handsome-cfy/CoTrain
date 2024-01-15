#ifndef __CoTrain_UNIQUEID_H__
#define __CoTrain_UNIQUEID_H__

#include<memory>
#include"../timer.h"


namespace CoTrain{

class UniqueID{
public:
    typedef std::shared_ptr<UniqueID> ptr;

    //将信息编码为id
    virtual char* encode() = 0;
    //从id中解码出信息，不一定必须实现
    virtual void decode() = 0;

    char* getid(){return m_id;}

    UniqueID(){}
    virtual ~UniqueID() = 0;
protected:
    //用于存储id编码的字符数组
    char* m_id;
    //最大长度
    uint16_t max_len;
};

class SnowFlakeID : public UniqueID{
public:
    char* encode() override;
    void decode() override;
    SnowFlakeID(uint64_t _machineID):
        machineId(_machineID),
        sequence(0),
        lastTimestamp(0){}

private:
    //机器id
    uint64_t machineId;
    //序列号
    uint64_t sequence;
    //时间戳
    uint64_t lastTimestamp;
};
}
#endif