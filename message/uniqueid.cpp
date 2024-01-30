#include "uniqueid.h"

char *CoTrain::SnowFlakeID::encode()
{
    uint64_t currentTimestamp = getTimeStamp();

    // Generate Snowflake ID
    uint64_t snowflakeId = ((currentTimestamp - epoch) << timestampShift) |
                           (workerId << workerIdShift) |
                           (sequence & sequenceMask);

    // Store the Snowflake ID in the m_id buffer
    snprintf(m_id, idBufferSize, "%llu", snowflakeId);

    return m_id;
}

void CoTrain::SnowFlakeID::decode()
{
}

uint64_t CoTrain::SnowFlakeID::getTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return static_cast<uint64_t>(millis);
}
