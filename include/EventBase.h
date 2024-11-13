#ifndef EVENT_BASE_H
#define EVENT_BASE_H

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <thread>
#include <any>
#include "Reflection.h"
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

    virtual void invokeEvent(const std::string& event_name, const std::vector<std::any>& args) = 0;

protected:
    // 事件处理器列表
    std::vector<std::function<void()>> handlers_;
    std::thread::id thread_id_;
};

#endif // EVENT_BASE_H