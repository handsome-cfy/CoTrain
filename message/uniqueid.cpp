#include "uniqueid.h"

namespace CoTrain{
uint64_t CoTrain::SnowFlakeID::encode()
{
    uint64_t currentTimestamp = getTimeStamp();

    // Generate Snowflake ID
    uint64_t snowflakeId = ((currentTimestamp - epoch) << timestampShift) |
                           (workerId << workerIdShift) |
                           (sequence & sequenceMask);

    // Store the Snowflake ID in the m_id buffer
    snprintf(m_id, idBufferSize, "%llu", snowflakeId);

    return snowflakeId;
}

void CoTrain::SnowFlakeID::decode()
{
    // 解析 Snowflake ID
    uint64_t snowflakeId = std::stoull(m_id);

    // 提取时间戳、工作节点 ID 和序列号
    timestamp = (snowflakeId >> timestampShift) + epoch;
    workerId = (snowflakeId >> workerIdShift) & ((1ULL << workerIdBits) - 1);
    sequence = snowflakeId & sequenceMask;
    
}

void SnowFlakeID::decode(uint64_t code)
{
        // 解析 Snowflake ID
    uint64_t snowflakeId = code;

    // 提取时间戳、工作节点 ID 和序列号
    timestamp = (snowflakeId >> timestampShift) + epoch;
    workerId = (snowflakeId >> workerIdShift) & ((1ULL << workerIdBits) - 1);
    sequence = snowflakeId & sequenceMask;
}

uint64_t CoTrain::SnowFlakeID::getTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return static_cast<uint64_t>(millis);
}

CoTrain::UniqueID::~UniqueID()
{
}

UniqueID::ptr CoTrain::UniqueIDMananger::generateID()
{
    UniqueID::ptr id = nullptr;
    switch (m_type)
    {
    case IDtype::SnowFlake:
        id = SnowFlakeID::ptr(new SnowFlakeID(m_workerId));
        break;
    
    default:
        break;
    }
    return id;
}

}