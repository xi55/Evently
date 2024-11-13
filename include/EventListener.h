
#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <thread>
#include <atomic>
#include <string>
#include <any>           
#include <vector>     
#include "EventBase.h"  
#include "nss/nano_signal_slot.hpp"  // 信号槽库，用于替代 Q_SIGNAL 和 Q_SLOT 功能
namespace Evently
{
    template <class Derived>
    class EventListener : public EventBase {
    public:
        void invokeEvent(const std::string& event_name, const std::vector<std::any>& args) override {
            auto method_name = "event_" + event_name;

            // 使用 SFINAE 检查 Derived 类是否有对应的成员函数
            if constexpr (std::is_member_function_pointer_v<decltype(&Derived::event_testEvent)>) {
                if (method_name == "event_testEvent") {
                    static_cast<Derived*>(this)->event_testEvent(args);
                }
            }
            // todo: 扩展添加更多事件类型
            // 
        }
    };

}
#endif // EVENTLISTENER_H