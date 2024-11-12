#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
#include <iostream>

class ThreadPool 
{
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    // 添加任务到线程池
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    void wait_for_all_tasks();

private:
    std::vector<std::thread> workers; // 工作线程
    std::queue<std::function<void()>> tasks; // 任务队列

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable all_tasks_done; // 任务完成条件变量
    std::atomic<bool> stop;
    std::atomic<size_t> unfinished_tasks; // 记录未完成任务数

    void worker_thread(); // 工作线程执行函数

    // 日志记录函数
    void log(const std::string& message);
};


template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> 
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop) 
        {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.emplace([task]() { (*task)(); });
        ++unfinished_tasks; // 任务计数加一
    }
    // log("Task enqueued.");
    condition.notify_one();
    return res;
}