#ifndef __CoTrain_LOG_H__
#define __CoTrain_LOG_H__

#include<string>
#include<stdint.h>
#include<memory>
#include<list>
#include<iostream>
#include<sstream>
#include<ctime>
#include<unistd.h>
#include<mutex>

#include"thread.h"

namespace CoTrain{


class LogEvent{
friend class LogFormatter;
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(){};
    LogEvent(std::string threadname){
        m_threadname = threadname;
        LogEvent();
    }

    uint32_t getThreadId(){return m_threadId;}
    uint32_t gettime(){return m_time;}
    std::string getcontent(){return m_content;}
    std::string getThreadName(){return m_threadname;}

    void setThreadId(uint32_t val){m_threadId = val;}
    void settime(uint32_t val){m_time = val;}
    void setcontent(std::string val){m_content = val;}
    void setThreadName(std::string val){m_threadname = val;}

protected: 
    
    const char* p_file = nullptr;
    int32_t m_line = 0;
    uint32_t m_threadId = 0;
    uint32_t m_fiberId = 0;
    uint64_t m_time;
    std::string m_content;

    //线程名称
    std::string m_threadname;
};

class LogLevel
{
public:
    enum Level{
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

};

class LogFormatter{
public: 
    typedef std::shared_ptr<LogFormatter> ptr;
    virtual std::string format(LogEvent::ptr event) = 0;
    virtual ~LogFormatter(){};
    // virtual LogFormatter(){};
};

class StdLogFormatter:public LogFormatter{
public:
    typedef std::shared_ptr<StdLogFormatter> ptr;
    std::string format(LogEvent::ptr event) override;
    
private:
};

class Logger;

class LogAppender{
friend class Logger;

public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){};
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
    
    LogLevel::Level getLevel(){return m_level;}
    void setLevel(LogLevel::Level val){m_level = val;}

    LogFormatter::ptr getFormatter(){return m_formatter;}
    void setFormatter(LogFormatter::ptr val){m_formatter = val;}

    std::string getLevelString(LogLevel::Level val);
protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

class Logger{
public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name="root");

    void log(LogLevel::Level level, LogEvent::ptr event);
    // 操作appenderlist
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    LogLevel::Level getLevel(){return m_level;};
    void setLevel(LogLevel::Level val);
private:
    std::string m_name;
    LogLevel::Level m_level;
    std::list<LogAppender::ptr> m_appenderlist;
};


class StdOutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdOutLogAppender> ptr;
    void log(LogLevel::Level level, LogEvent::ptr event) override;
private:  

};


class LogManager{
public:
    typedef std::shared_ptr<LogManager> ptr;

    static ptr instance(){
        static ptr m_instance;
        if(!m_instance){
            m_instance = ptr(new LogManager());
            m_instance->init();
        }

        return m_instance;
    }

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void init();
    void init(LogLevel::Level level){
        init();
        setLevel(level);
    }
    void setLevel(LogLevel::Level level);
    LogEvent::ptr CreateEvent(std::string content);
    LogEvent::ptr CreateEvent(std::string content,std::string threadname)
    {
        LogEvent::ptr ptr = CreateEvent(content);
        ptr->setThreadName(threadname);
        return ptr;
    }
    ~LogManager();
private:
    //单例设计模式
    LogManager(){
    };
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&)=delete;
    
    // static ptr m_instance;
    //互斥锁
    std::mutex m_log_mutex;

    // std::mutex m_queue_mutex;
    std::queue<Task> m_log_queue;
    
    Thread::ptr m_log_thread;

    Logger::ptr m_logger;
    LogLevel::Level m_level;
    time_t m_init_time;

    bool m_stop=false;
};
// LogMannager::ptr LogMannager::m_instance = nullptr;

}
#endif