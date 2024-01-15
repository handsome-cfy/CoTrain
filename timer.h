#ifndef __CoTrain_TIMER_H__
#define __CoTrain_TIMER_H__

#include<chrono>
#include<memory>
#include<mutex>

namespace CoTrain{

class Timer {
private:
    std::chrono::steady_clock::time_point startTime;

    Timer() {
        startTime = std::chrono::steady_clock::now();
    }

public:
    typedef std::shared_ptr<Timer> ptr;

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    static Timer& getInstance() {
        static Timer instance;
        return instance;
    }

    double getTimeInSeconds() const {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration<double>(currentTime - startTime);
        return elapsedTime.count();
    }

    uint64_t getTimeInMilliSeconds() const{
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration<double>(currentTime - startTime);
        auto output = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime);
        return output.count();
    }
};

}
#endif