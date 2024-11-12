#include <iostream>
#include <memory>
#include <thread>
#include "Application.h"
#include "EventBase.h"

// 自定义事件类，用于测试
class TestEvent : public EventBase {
public:
    std::string getEventName() const override {
        return "TestEvent";
    }

    void trigger(const std::vector<std::any>& args) override {
        std::cout << "事件触发，参数:";
        for (const auto& arg : args) {
            if (arg.type() == typeid(int)) {
                std::cout << " " << std::any_cast<int>(arg);
            } else if (arg.type() == typeid(std::string)) {
                std::cout << " " << std::any_cast<std::string>(arg);
            }
        }
        std::cout << std::endl;
    }
};

int main() {
    Evently::Application<int, std::string> app;

    // 测试用例 1：事件订阅和触发
    {
        auto event = std::make_shared<TestEvent>();
        
        // 订阅事件
        app.subscribeEvent("TestEvent", event);

        // 发布事件
        std::cout << "测试用例 1：发布事件 'TestEvent'，参数为 42 和 '你好'" << std::endl;
        app.publishEvent("TestEvent", std::launch::async, 42, std::string("你好"));
    }

    // 测试用例 2：事件取消订阅
    {
        auto event = std::make_shared<TestEvent>();
        
        // 订阅事件
        app.subscribeEvent("TestEvent", event);

        // 取消订阅事件
        app.unsubscribeEvent("TestEvent", event);

        // 发布事件，不应触发
        std::cout << "测试用例 2：取消订阅 'TestEvent' 后再次发布，期望不触发" << std::endl;
        app.publishEvent("TestEvent", std::launch::async, 42, std::string("你好"));
    }

    // // 测试用例 3：多优先级事件处理
    // {
    //     class PriorityEvent : public EventBase {
    //     public:
    //         PriorityEvent(const std::string& name, int priority) : name_(name), priority_(priority) {}

    //         std::string getEventName() const override {
    //             return "PriorityEvent";
    //         }

    //         void trigger(const std::vector<std::any>& args) override {
    //             std::cout << "优先级 " << priority_ << " 事件触发，名称: " << name_ << std::endl;
    //         }

    //     private:
    //         std::string name_;
    //         int priority_;
    //     };

    //     auto highPriorityEvent = std::make_shared<PriorityEvent>("高优先级", 0);
    //     auto normalPriorityEvent = std::make_shared<PriorityEvent>("普通优先级", 50);
    //     auto lowPriorityEvent = std::make_shared<PriorityEvent>("低优先级", 100);

    //     // 按优先级顺序订阅事件
    //     app.subscribeEvent("PriorityEvent", lowPriorityEvent, 100);
    //     app.subscribeEvent("PriorityEvent", normalPriorityEvent, 50);
    //     app.subscribeEvent("PriorityEvent", highPriorityEvent, 0);

    //     // 发布事件
    //     std::cout << "测试用例 3：发布 'PriorityEvent' 事件，按优先级触发" << std::endl;
    //     app.publishEvent("PriorityEvent", std::launch::async);
    // }

    // 测试用例 4：多线程事件处理
    // {
    //     auto event1 = std::make_shared<TestEvent>();
    //     auto event2 = std::make_shared<TestEvent>();

    //     // 订阅事件
    //     app.subscribeEvent("MultiThreadEvent", event1);
    //     app.subscribeEvent("MultiThreadEvent", event2);

    //     // 多线程发布事件
    //     std::cout << "测试用例 4：多线程环境下发布 'MultiThreadEvent' 事件" << std::endl;
    //     std::thread t1([&]() { app.publishEvent("MultiThreadEvent", std::launch::async, 100); });
    //     std::thread t2([&]() { app.publishEvent("MultiThreadEvent", std::launch::async, 200); });

    //     t1.join();
    //     t2.join();
    // }

    return 0;
}
