#ifndef __CoTrain_THREAD_H__
#define __CoTrain_THREAD_H__

#include <thread>
#include <memory>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <string>
#include <sstream>

namespace CoTrain
{
    class Thread
    {
    public:
        typedef std::shared_ptr<Thread> ptr;
        template <typename Callable, typename... Args>
        explicit Thread(std::string name, Callable &&func, Args &&...args)
            : m_name(name), m_thread(std::forward<Callable>(func),
                                     std::forward<Args>(args)...) {}

        void join() { m_thread.join(); }

    protected:
    private:
        std::thread m_thread;
        std::string m_name;
    };

    class Semaphore
    {
    public:
        typedef std::shared_ptr<Semaphore> ptr;
        explicit Semaphore(int32_t count = 0) : m_count(count) {}

        // notify
        void notify();
        //
        void wait();
        
        //尽可能不使用
        void notify_all();

    protected:
    private:
        int32_t m_count;
        std::mutex m_mutex;
        std::condition_variable m_condition;
    };

    class Task
    {
    public:
        int priority;
        std::function<void()> callback;
        template <typename Callable, typename... Args>
        Task(int priority, Callable &&callable, Args &&...args)
            : priority(priority),
              callback(std::bind(std::forward<Callable>(callable), std::forward<Args>(args)...)) {}
    };

    class TaskCompare
    {
    public:
        bool operator()(const Task &task1, const Task &task2)
        {
            return task1.priority < task2.priority;
        }
    };

    class ThreadPool
    {
    public:
        typedef std::shared_ptr<ThreadPool> ptr;

        // 初始化线程池
        void init();
        void init(int16_t maxnumber)
        {
            m_max_thread_number = maxnumber;
            init();
        }

        // 加入任务队列
        void enqueue(const Task& task);

        ThreadPool(){
            m_max_thread_number = 20;
            m_stop = false;
        };
        ~ThreadPool();

    private:
        // 最大线程数量
        int16_t m_max_thread_number = 20;
        // 任务队列
        std::priority_queue<Task, std::vector<Task>, TaskCompare> m_taskqueue;
        //用于互斥访问任务队列的互斥锁
        std::mutex m_task_mutex;
        // 线程池
        std::vector<Thread::ptr> m_pool;

        Semaphore::ptr m_pool_sem;

        bool m_stop=false;

        // 禁止拷贝构造
        ThreadPool(const ThreadPool &) = delete;
        ThreadPool &operator=(const ThreadPool &) = delete;
    };

}
#endif