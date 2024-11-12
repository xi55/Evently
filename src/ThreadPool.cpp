#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t num_threads) : stop(false), unfinished_tasks(0)
{
    for (size_t i = 0; i < num_threads; ++i) 
    {
        workers.emplace_back([this] { this->worker_thread(); });
    }
    // log("ThreadPool initialized with " + std::to_string(num_threads) + " threads.");
}

ThreadPool::~ThreadPool() 
{
    stop = true;
    condition.notify_all();
    for (auto& worker : workers) 
    {
        if (worker.joinable()) 
        {
            worker.join();
        }
    }
    // log("ThreadPool destroyed.");
}

void ThreadPool::worker_thread() 
{
    while (!stop) 
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) 
            {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        // log("Task started.");
        task();
        // log("Task completed.");
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (--unfinished_tasks == 0) 
            {
                all_tasks_done.notify_all(); // 所有任务完成时通知
            }
        }
    }
}

void ThreadPool::wait_for_all_tasks() 
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    all_tasks_done.wait(lock, [this] { return unfinished_tasks == 0 && tasks.empty(); });
}

void ThreadPool::log(const std::string& message) 
{
    std::lock_guard<std::mutex> lock(queue_mutex); // 保证日志输出的线程安全
    std::cout << "[ThreadPool] " << message << std::endl;
}


