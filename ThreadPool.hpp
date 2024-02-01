#pragma once

#include <queue>
#include <future>
#include <condition_variable>
#include <vector>
#include <thread>
#include <mutex>

void TaskFunc(int id, int delay);
typedef std::function<void()> task_type;
typedef void (*FuncType) (int, int);
typedef void (*FuncTypeBool) (bool, bool, bool);
typedef void (*FuncTypeArray) (int*, long, long);


template<class T>
class BlockedQueue
{
    private:
        std::mutex m_locker;
        std::queue<T> m_task_queue;
        std::condition_variable m_notifier;

    public:

        void push(T& item)
        {
            std::lock_guard<std::mutex> lock(m_locker);

            m_task_queue.push(item);

            m_notifier.notify_one();
        }
        void pop(T& item)
        {
            std::unique_lock<std::mutex> u_lock(m_locker);
            if (m_task_queue.empty())
            {
                m_notifier.wait(u_lock, [this]{return !m_task_queue.empty();});
            }

            item = m_task_queue.front();
            m_task_queue.pop();
        }
        bool fast_pop(T& item)
        {
            std::lock_guard<std::mutex> lock(m_locker);

            if (m_task_queue.empty())
            {
                return false;
            }
            item = m_task_queue.front();
            m_task_queue.pop();
            return true;
        }
};

class ThreadPoolMod
{
    public:
        ThreadPoolMod();
        void start();
        void stop();
        void push_task(FuncType f, int id, int arg);
        void push_task(FuncTypeArray f, int* array, long left, long right);
        void threadFunc(int qindex);

    private:
        int m_thread_count;
        std::vector<std::thread> m_threads;
        std::vector<BlockedQueue<task_type>> m_thread_queues;
        int m_index;
};

class RequestHandler
{
   public:
       RequestHandler();
       ~RequestHandler();
       // отправка запроса на выполнение
       void pushRequest(FuncTypeArray f, int* array, long left, long right);
   private:
       // пул потоков
       ThreadPoolMod m_tpool;
};