#include "ThreadPool.hpp"

ThreadPoolMod::ThreadPoolMod() : m_thread_count(std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency():4),
m_thread_queues(m_thread_count) {}

void ThreadPoolMod::start()
{
    for (int i = 0; i < m_thread_count; i++)
    {
        m_threads.emplace_back(&ThreadPoolMod::threadFunc, this, i);
    }
}

void ThreadPoolMod::stop()
{
    for(int i=0;i<m_thread_count; i++)
    {
       // кладем задачу-пустышку в каждую очередь
       // для завершения потока
       task_type empty_task;
       m_thread_queues[i].push(empty_task);
   }
   for(auto& t: m_threads)
   {
       t.join();
   }
}

void ThreadPoolMod::push_task(FuncType f, int id, int arg)
{
    int queue_to_push = m_index++ % m_thread_count;
   // формируем функтор
   task_type task = [=]{f(id, arg);};
   // кладем в очередь
   m_thread_queues[queue_to_push].push(task);
}

std::future<void> ThreadPoolMod::push_task(FuncTypeArray f, int *array, long left, long right)
{
    int queue_to_push = m_index++ % m_thread_count;
    auto promise = std::make_shared<std::promise<void>>();
    auto result = promise->get_future();
    task_type task = ([=]{f(array, left, right); promise->set_value();});
   // кладем в очередь
    m_thread_queues[queue_to_push].push(task);
    return result;
}

void ThreadPoolMod::threadFunc(int qindex)
{
    while(true)
    {
        task_type task_to_do;
        bool res;
        int i = 0;
        for(; i < m_thread_count; i++)
        {
            // попытка быстро забрать задачу из любой очереди, начиная со своей
            if(res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }

        if (!res)
        {
            // вызываем блокирующее получение очереди
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do)
        {
            // чтобы не допустить зависания потока
            // кладем обратно задачу-пустышку
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (!task_to_do)
        {
            return;
        }
        // выполняем задачу
        task_to_do();
    }
}

RequestHandler::RequestHandler()
{
   m_tpool.start();
}
RequestHandler::~RequestHandler()
{
   m_tpool.stop();
}
std::future<void> RequestHandler::pushRequest(FuncTypeArray f, int* array, long left, long right)
{
   return m_tpool.push_task(f, array, left, right);
}
