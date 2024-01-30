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
    uint16_t max_id_len;
    //最大长度
    uint16_t max_buf_len;
};

class SnowFlakeID : public UniqueID{
public:
    char* encode() override;
    void decode() override;
    SnowFlakeID(uint64_t _machineID):
        workerId(_machineID),
        sequence(0),
        lastTimestamp(0){
        }

    uint64_t getTimeStamp();

private:
    //机器id
    uint64_t workerId;
    //序列号
    uint64_t sequence;
    //时间戳
    uint64_t lastTimestamp;
public:
    constexpr static uint64_t epoch = 1609459200000; // Epoch time (in milliseconds) for Snowflake ID (e.g., January 1, 2021)
    constexpr static int timestampBits = 41; // Number of bits allocated for timestamp
    constexpr static int workerIdBits = 10; // Number of bits allocated for worker ID
    constexpr static int sequenceBits = 12; // Number of bits allocated for sequence
    constexpr static int workerIdShift = sequenceBits; // Number of bits to shift for worker ID
    constexpr static int timestampShift = sequenceBits + workerIdBits; // Number of bits to shift for timestamp
    constexpr static uint64_t sequenceMask = (1ULL << sequenceBits) - 1; // Mask to extract sequence bits
    constexpr static int idBufferSize = 20; // Buffer size for storing the Snowflake ID as a string
};
}
#endif