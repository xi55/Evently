#ifndef EVENT_BASE_H
#define EVENT_BASE_H

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <thread>
#include <any>
#include "Reflection.h"
#include <cxxabi.h>
#include <iostream>
#define FUNCNAME_PREFIX "event_"

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

    virtual void invokeEvent(const std::string& event_name, const std::vector<std::any>& args)
    {
        std::string method_name = FUNCNAME_PREFIX + event_name;
        auto& registry = Evently::ReflectionRegistry::getInstance();
        // 使用 demangleTypeName 解码类名
        std::string class_name = demangleTypeName(typeid(*this));
        try
        {
            
            registry.invokeMethod(class_name, method_name, this, args);
            // std::cout << "Event " << event_name << " invoked method " << method_name << "\n";
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }

protected:
    // 事件处理器列表
    std::vector<std::function<void()>> handlers_;
    std::thread::id thread_id_;
    // 类型解码方法，只能在派生类中使用
    static std::string demangleTypeName(const std::type_info& typeInfo) 
    {
#if defined(__linux__)
        int status;
        char* demangled = abi::__cxa_demangle(typeInfo.name(), nullptr, nullptr, &status);
        std::string result = (status == 0 && demangled) ? demangled : typeInfo.name();
        free(demangled);
        return result;
#else
        return typeInfo.name();
#endif
    }

    // getList 方法，用于向 typeNames 和 list 中添加类型信息和对象指针
    template<class T>
    static void getList(std::vector<std::string>& typeNames, std::vector<std::shared_ptr<void>>& list, T&& t) {
        // 添加解码后的类型名
        typeNames.push_back(demangleTypeName(typeid(t)));
        // 存储对象指针
        list.push_back(std::make_shared<T>(std::forward<T>(t)));
    }

};

template <typename Derived>
class SharedEventBase : public EventBase, public std::enable_shared_from_this<Derived> {
public:
    // 可以在基类中定义公共的功能和接口
    virtual ~SharedEventBase() = default;
};

#endif // EVENT_BASE_H