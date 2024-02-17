#ifndef __CoTrain_LOG_CPP__
#define __CoTrain_LOG_CPP__

#include "log.h"
namespace CoTrain
{

    CoTrain::Logger::Logger(const std::string &name)
        : m_name(name)
    {
    }

    void CoTrain::Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            for (auto &it : m_appenderlist)
            {
                it->log(level, event);
            }
        }
    }

    void CoTrain::Logger::addAppender(LogAppender::ptr appender)
    {
        if(appender != nullptr)
            m_appenderlist.push_back(appender);
    }

    void CoTrain::Logger::delAppender(LogAppender::ptr appender)
    {
        if (!appender)
        {
            return;
        }
        for (auto it = m_appenderlist.begin();
             it != m_appenderlist.end();
             it++)
        {
            if (*it == appender)
            {
                m_appenderlist.erase(it);
            }
        }
    }

    void Logger::setLevel(LogLevel::Level val)
    {
        m_level = val;
        for (auto &it : m_appenderlist)
        {
            it->setLevel(m_level);
        }
    }

    void CoTrain::StdOutLogAppender::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            std::cout << getLevelString(level) << " " << m_formatter->format(event) << std::endl;
        }
    }

    std::string CoTrain::StdLogFormatter::format(LogEvent::ptr event)
    {
        std::stringstream ss;
        if(event->getThreadName().empty()){
            ss << "ThreadId: " << event->getThreadId()
           << " Time: " << event->gettime()
           << " Content: " << event->getcontent();
        }else{
            ss << "ThreadId: " << event->getThreadId()
           << " ThreadName: " << event->getThreadName()
           << " Time: " << event->gettime()
           << " Content: " << event->getcontent();
        }

        return ss.str();
    }

#endif

    std::string CoTrain::LogAppender::getLevelString(LogLevel::Level val)
    {
        switch (val)
        {
        case 1:
            return "DEBUG";
        case 2:
            return "INFO";
        case 3:
            return "WARN";
        case 4:
            return "ERROR";
        case 5:
            return "FATAL";
        }
    }

    void LogManager::debug(LogEvent::ptr event)
    {
        std::unique_lock<std::mutex> lock(m_log_mutex);
        
        m_log_queue.push(Task(1, [this,event]()
        {
        m_logger->log(LogLevel::Level::DEBUG, event);
        }));

    }

    void LogManager::info(LogEvent::ptr event)
    {
        std::unique_lock<std::mutex> lock(m_log_mutex);

        m_log_queue.push(Task(1, [this,event]()
        {
        m_logger->log(LogLevel::Level::INFO, event);
        }));
    }

    void LogManager::warn(LogEvent::ptr event)
    {
        std::unique_lock<std::mutex> lock(m_log_mutex);

        m_log_queue.push(Task(1, [this,event]()
        {
        m_logger->log(LogLevel::Level::WARN, event);
        }));
    }

    void LogManager::error(LogEvent::ptr event)
    {
        std::unique_lock<std::mutex> lock(m_log_mutex);

        m_log_queue.push(Task(1, [this,event]()
        {
        m_logger->log(LogLevel::Level::ERROR, event);
        }));
    }

    void LogManager::fatal(LogEvent::ptr event)
    {
        std::unique_lock<std::mutex> lock(m_log_mutex);

        m_log_queue.push(Task(1, [this,event]()
        {
        m_logger->log(LogLevel::Level::FATAL, event);
        }));
    }

    void CoTrain::LogManager::init()
    {
        m_init_time = time(nullptr);
        m_level = LogLevel::Level::DEBUG;

        Logger *tmp_ptr = new Logger("");
        m_logger = Logger::ptr(tmp_ptr);
        m_logger->setLevel(m_level);

        // 实例化appender
        StdLogFormatter::ptr formatter = StdLogFormatter::ptr(new StdLogFormatter());
        LogAppender::ptr appender = StdOutLogAppender::ptr(new StdOutLogAppender());
        appender->setFormatter(formatter);
        appender->setLevel(m_level);
        m_logger->addAppender(appender);

        // 初始化一个线程，专门用于输出log
        m_log_thread = Thread::ptr(new Thread("LogerThread",
                                              [this]()
                                              {
                                                  while (true)
                                                  {
                                                      {
                                                          {
                                                              std::unique_lock<std::mutex> lock(m_log_mutex);
                                                              if (!m_log_queue.empty())
                                                              {
                                                                  Task task = m_log_queue.front();
                                                                  m_log_queue.pop();
                                                                  task.callback();
                                                              }
                                                              if (m_stop && m_log_queue.empty())
                                                                  return;
                                                          }
                                                      }
                                                  }
                                              }));

        // 初始化第一个event
        info(CreateEvent("LoggerInited"));
    }

    void LogManager::setLevel(LogLevel::Level level)
    {
        m_logger->setLevel(level);
    }

    LogEvent::ptr CoTrain::LogManager::CreateEvent(std::string content)
    {
        LogEvent::ptr event = LogEvent::ptr(new LogEvent());
        event->setThreadId(getpid());
        event->settime(time(nullptr) - m_init_time);
        event->setcontent(content);
        return event;
    }
    LogManager::~LogManager()
    {
        {
            std::unique_lock<std::mutex> lock(m_log_mutex);
            m_stop=true;
        }
        if(m_log_thread != nullptr){
            m_log_thread->join();
        }
    }
}
