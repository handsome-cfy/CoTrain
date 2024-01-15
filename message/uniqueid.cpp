#include "uniqueid.h"

char *CoTrain::SnowFlakeID::encode()
{
    Timer& timer = Timer::getInstance();
    if(lastTimestamp < timer.getTimeInMilliSeconds()){
        sequence = (sequence + 1) & 0x0fff;
        if (sequence == 0){
            lastTimestamp = timer.getTimeInMilliSeconds();
        }
    }
    return m_id;
}

void CoTrain::SnowFlakeID::decode()
{
}
