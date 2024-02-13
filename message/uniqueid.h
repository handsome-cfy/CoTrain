#ifndef __CoTrain_UNIQUEID_H__
#define __CoTrain_UNIQUEID_H__

#include<memory>
#include"../timer.h"


namespace CoTrain{

class UniqueID{
public:
    typedef std::shared_ptr<UniqueID> ptr;

    //将信息编码为id
    virtual uint64_t encode() = 0;
    //从id中解码出信息，不一定必须实现
    virtual void decode() = 0;

    char* getid(){return m_id;}

    UniqueID(){}
    
    virtual ~UniqueID();
protected:
    //用于存储id编码的字符数组
    char* m_id;
    uint16_t max_id_len;
    //最大长度
    uint16_t max_buf_len;
};

class SnowFlakeID : public UniqueID{
public:
    uint64_t encode() override;
    void decode() override;
    void decode(uint64_t code);
    SnowFlakeID(){}
    SnowFlakeID(uint64_t _machineID):
        workerId(_machineID),
        sequence(0),
        lastTimestamp(0){
        m_id = (char*)std::malloc(sizeof(uint64_t));
        }
    
    ~SnowFlakeID(){}
    uint64_t getTimeStamp();

        // 设置和获取函数
    void setWorkerId(uint64_t id) {
        workerId = id;
    }

    uint64_t getWorkerId() const {
        return workerId;
    }

    void setSequence(uint64_t seq) {
        sequence = seq;
    }

    uint64_t getSequence() const {
        return sequence;
    }

    void setLastTimestamp(uint64_t timestamp) {
        lastTimestamp = timestamp;
    }

    uint64_t getLastTimestamp() const {
        return lastTimestamp;
    }

private:
    //机器id
    uint64_t workerId;
    //序列号
    uint64_t sequence;
    //时间戳
    uint64_t lastTimestamp;
    
    uint64_t timestamp;
public:
    constexpr static uint64_t epoch = 1609459200000; // Epoch time (in milliseconds) for Snowflake ID (e.g., January 1, 2021)
    constexpr static int timestampBits = 41; // Number of bits allocated for timestamp
    constexpr static int workerIdBits = 10; // Number of bits allocated for worker ID
    constexpr static int sequenceBits = 12; // Number of bits allocated for sequence
    constexpr static int workerIdShift = sequenceBits; // Number of bits to shift for worker ID
    constexpr static int timestampShift = sequenceBits + workerIdBits; // Number of bits to shift for timestamp
    constexpr static uint64_t sequenceMask = (1ULL << sequenceBits) - 1; // Mask to extract sequence bits
    constexpr static int idBufferSize = sizeof(uint64_t); // Buffer size for storing the Snowflake ID as a string
};


class UniqueIDMananger{
public:
    typedef std::shared_ptr<UniqueIDMananger> ptr;

    enum IDtype{
        SnowFlake,
        Default
    };

    UniqueIDMananger(){}
    UniqueIDMananger(IDtype type, uint64_t _workerId)
        :m_type(type),m_workerId(_workerId){}

    UniqueID::ptr generateID();
private:
    uint64_t m_workerId;
    IDtype m_type=IDtype::SnowFlake;
};
}
#endif