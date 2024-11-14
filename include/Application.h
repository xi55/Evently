#ifndef APPLICATION_H
#define APPLICATION_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <mutex>
#include <thread>
#include <cassert>
#include "EventBase.h"
#include <queue>
#include <any>
#include <functional>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#if defined(__linux__)
    #include <cxxabi.h>
#endif
#include <vector>
#include <shared_mutex>
#include <future>

namespace Evently
{
    
    template <class... Args>
    class Application
    {
    public:

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        // 单例访问接口
        static Application& getInstance() {
            static Application instance;
            return instance;
        }

        // 注册事件监听器
        void subscribeEvent(const std::string& evently_name, std::shared_ptr<EventBase> listener, int priority = 50) 
        {
            std::unique_lock<std::shared_mutex> lock(event_mutex_);
            try
            {
                auto& pool = getEventPool(priority);
                pool[evently_name].push_back(listener);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            
        }

        // 取消订阅特定事件监听器
        void unsubscribeEvent(const std::string& evently_name, std::shared_ptr<EventBase> listener) 
        {
            std::unique_lock<std::shared_mutex> lock(event_mutex_);

            removeListenerFromPool(event_pool_high_, evently_name, listener);
            removeListenerFromPool(event_pool_, evently_name, listener);
            removeListenerFromPool(event_pool_low_, evently_name, listener);
        }

        // 取消订阅所有事件
        void unsubscribeAll(std::shared_ptr<EventBase> listener) 
        {
            std::unique_lock<std::shared_mutex> lock(event_mutex_);
            removeAllOccurrences(event_pool_high_, listener);
            removeAllOccurrences(event_pool_, listener);
            removeAllOccurrences(event_pool_low_, listener);
        }

        template <class... EventArgs>
        void publishEvent(const std::string &evently_name, std::launch policy, EventArgs&&... args)
        {
            
            if (show_detailed_) 
            {
                std::cout << "\n"
                        << "--------------发布事件：-----------\n"
                        << "事件名称: " << evently_name << "\n"
                        << "事件策略: " << (policy == std::launch::async ? "异步" : "延迟") << "\n"
                        << "事件线程: " << std::this_thread::get_id() << "\n"
                        << "----------------------------------\n";
            }
            if (evently_name.empty()) return;
            auto numArgs = sizeof...(args);

            assert(numArgs <= 10 && "事件参数数量不得超过10");

            std::unique_lock<std::mutex> lock(event_queue_mutex_);
            std::cout << "---- event_pool_high_ ----" << event_pool_high_.size() << std::endl;
            std::cout << "---- event_pool_ ----" << event_pool_.size() << std::endl;
            std::cout << "---- event_pool_low_ ----" << event_pool_low_.size() << std::endl;
            if (event_pool_high_.count(evently_name) ||
                event_pool_.count(evently_name)      ||
                event_pool_low_.count(evently_name))
            {
                auto handlers = event_pool_high_[evently_name];
                handlers.insert(handlers.end(), event_pool_[evently_name].begin(), event_pool_[evently_name].end());
                handlers.insert(handlers.end(), event_pool_low_[evently_name].begin(), event_pool_low_[evently_name].end());
                auto methodName = std::string(FUNCNAME_PREFIX) + evently_name;
                std::cout << "---- handlers ----" << handlers.size() << std::endl;
                for (auto& handler : handlers)
                {
                    if (handler) 
                    {
                        std::cout << "处理器: " << handler.get() << "\n";
                    }
                    else 
                    {
                        std::cout << "---------------\n";
                    }
                    // todo: 改进同步策略，本意为相同线程且异步策略下，阻塞运行。
                    if (std::this_thread::get_id() == handler->getThreadId() || policy != std::launch::async) 
                    {
                        std::cout << "直接运行\n";
                        invokeDirect(evently_name, handler, std::forward<EventArgs>(args)...);
                    } 
                    else 
                    {
                        std::cout << "子线程运行\n";
                        queueEvent(evently_name, handler, std::forward<EventArgs>(args)...);
                    }
                    
                }

            }
        }

    private:

        Application() : stop_processing_(false) {
            // 启动事件处理线程
            
            event_processing_thread_ = std::thread(&Application::processEvents, this);
            
        }

        ~Application() {
            // 停止事件处理线程
            stop_processing_ = true;
            event_cv_.notify_all();
            if (event_processing_thread_.joinable()) {
                event_processing_thread_.join();
            }
        }

        // 获取事件池
        std::unordered_map<std::string, std::vector<std::shared_ptr<EventBase>>>& getEventPool(int priority) {
            if (priority >= 100) {
                return event_pool_low_;
            } else if (priority > 0 && priority < 100) {
                return event_pool_;
            } else if (priority == 0) {
                return event_pool_high_;
            } else {
                throw std::invalid_argument("优先级必须为 0 或正整数");
            }
        }

        // 从指定池中移除监听器
        void removeListenerFromPool(std::unordered_map<std::string, std::vector<std::shared_ptr<EventBase>>>& pool,
                                    const std::string& evently_name, std::shared_ptr<EventBase> listener) {
            auto it = pool.find(evently_name);
            if (it != pool.end()) 
            {
                auto& listeners = it->second;
                listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
                if (listeners.empty()) {
                    pool.erase(it);
                }
            }
        }

        // 从所有事件池中移除指定监听器
        void removeAllOccurrences(std::unordered_map<std::string, std::vector<std::shared_ptr<EventBase>>>& pool,
                                  std::shared_ptr<EventBase> listener) 
        {
            for (auto it = pool.begin(); it != pool.end();) {
                auto& listeners = it->second;
                listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
                if (listeners.empty()) {
                    it = pool.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // 获取所有优先级的处理器列表
        std::vector<std::shared_ptr<EventBase>> getAllHandlers(const std::string& evently_name) {
            std::vector<std::shared_ptr<EventBase>> handlers;
            auto addHandlers = [&handlers](auto& pool, const std::string& name) {
                auto it = pool.find(name);
                if (it != pool.end()) {
                    handlers.insert(handlers.end(), it->second.begin(), it->second.end());
                }
            };
            addHandlers(event_pool_high_, evently_name);
            addHandlers(event_pool_, evently_name);
            addHandlers(event_pool_low_, evently_name);
            return handlers;
        }

        template <typename... CallArgs>
        void invokeDirect(const std::string &evently_name, std::shared_ptr<EventBase> eventHandler, CallArgs&&... callArgs) 
        {
            std::vector<std::any> arguments = {std::forward<CallArgs>(callArgs)...};
            // 直接调用，带有参数 (根据实际应用需求定制逻辑)
            std::cout << "直接调用，参数数量: " << arguments.size() << "\n";
            eventHandler->invokeEvent(evently_name, arguments);
        }
        
        template <typename... CallArgs>
        void queueEvent(const std::string &evently_name, std::shared_ptr<EventBase> eventHandler, CallArgs&&... callArgs) 
        {
            std::lock_guard<std::mutex> lock(event_queue_mutex_);
            

            // 使用完备参数包构造所需的三元组
            event_queue_.emplace(evently_name, eventHandler, std::make_any<std::tuple<std::decay_t<CallArgs>...>>(std::forward<CallArgs>(callArgs)...));

            event_cv_.notify_one();
        }


        // 事件处理线程
        void processEvents() 
        {
            while (!stop_processing_) 
            {
                std::unique_lock<std::mutex> lock(event_queue_mutex_);
                event_cv_.wait(lock, [this] { return !event_queue_.empty() || stop_processing_; });

                if (stop_processing_ && event_queue_.empty()) break;

                // 从队列中取出事件
                auto [evently_name, handler, args] = event_queue_.front();
                event_queue_.pop();
                lock.unlock();

                // 触发事件
                if (handler) 
                {
                    if (args.has_value()) 
                    {
                        if (args.type() == typeid(std::tuple<>)) 
                        {
                            handler->invokeEvent(evently_name, {}); // 无参数情况
                        } 
                        else 
                        {
                            // todo: 检测参数包
                            // 尝试将 std::any 转换为具体的 tuple 类型
                            auto& tuple = std::any_cast<const std::tuple<std::string>&>(args);
                            std::vector<std::any> argumentList = tupleToAnyVector(tuple);
                            // auto args_vector = tupleToAnyVector(*args_tuple);
                            handler->invokeEvent(evently_name, argumentList);
                        }
                    }
                }
                else
                {
                    std::cout << "Error: Handler for event " << evently_name << " is null.\n";
                }
            }
        }

        template <typename Tuple, std::size_t... Indexes>
        std::vector<std::any> tupleToAnyVectorImpl(const Tuple& tuple, std::index_sequence<Indexes...>) {
            return {std::any(std::get<Indexes>(tuple))...};
        }

        template <typename... TupleArgs>
        std::vector<std::any> tupleToAnyVector(const std::tuple<TupleArgs...>& tuple) {
            return tupleToAnyVectorImpl(tuple, std::index_sequence_for<TupleArgs...>{});
        }


        bool show_detailed_ = true;
        // 高优先级事件池
        std::unordered_map<std::string, std::vector<std::shared_ptr<EventBase>>> event_pool_high_;
        // 默认优先级事件池
        std::unordered_map<std::string, std::vector<std::shared_ptr<EventBase>>> event_pool_;
        // 低优先级事件池
        std::unordered_map<std::string, std::vector<std::shared_ptr<EventBase>>> event_pool_low_;
        // 事件池锁
        std::shared_mutex event_mutex_;

        // 跨线程事件队列
        std::queue<std::tuple<std::string, std::shared_ptr<EventBase>, std::any>> event_queue_;

        // std::queue<std::tuple<std::string, std::shared_ptr<EventBase>, std::tuple<Args...>>> event_queue_;
        std::mutex event_queue_mutex_;
        std::condition_variable event_cv_;

        std::thread event_processing_thread_;  // 事件处理线程
        std::atomic<bool> stop_processing_;    // 控制线程结束的标志
    };
}

#endif // APPLICATION_H