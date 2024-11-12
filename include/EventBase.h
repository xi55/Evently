#ifndef EVENT_BASE_H
#define EVENT_BASE_H

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <thread>
#include <any>
#define FUNCNAME "event_"

class EventBase
{
public:
    EventBase() : thread_id_(std::this_thread::get_id()) {};
    virtual ~EventBase() = default;


    // 获取事件所属的线程 ID
    std::thread::id getThreadId() const {
        return thread_id_;
    }

    // 获取事件名称
    virtual std::string getEventName() const = 0;

    // 注册事件处理器
    // void registerHandler(const std::function<void()>& handler) {
    //     handlers_.emplace_back(handler);
    // }

    // 触发事件：调用所有已注册的处理器
    virtual void trigger(const std::vector<std::any>& args) {
        
    }

protected:
    // 事件处理器列表
    std::vector<std::function<void()>> handlers_;
    std::thread::id thread_id_;
};

#endif // EVENT_BASE_H