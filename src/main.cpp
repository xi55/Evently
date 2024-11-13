#include <iostream>
#include <string>
#include <vector>
#include <any>
#include <memory>
#include <thread>
#include <chrono>
#include "Application.h"
#include "EventBase.h"
#include "Reflection.h"
#include "Evently.h" 

class MyListener : public EventBase {
public:
    MyListener() = default;
    std::string getEventName() const override { return "MyListener"; }

    void event_testEvent(const std::string& message) {
        std::cout << "处理事件 testEvent，消息: " << message << std::endl;
    }

    void event_numericEvent(int number, double value) {
        std::cout << "处理事件 numericEvent，整数: " << number << "，浮点数: " << value << std::endl;
    }

    void event_mixedEvent(const std::string& name, int age, bool status) {
        std::cout << "处理事件 mixedEvent，姓名: " << name << "，年龄: " << age << "，状态: " << status << std::endl;
    }
};

// 使用宏注册 MyListener 类和其方法
REGISTER_CLASS(MyListener)
REGISTER_METHOD(MyListener, event_testEvent)
REGISTER_METHOD(MyListener, event_numericEvent)
REGISTER_METHOD(MyListener, event_mixedEvent)

int main() {
    Evently::ReflectionRegistry& registry = Evently::ReflectionRegistry::getInstance();
    Evently::Application<> app;

    std::shared_ptr<MyListener> listener = std::make_shared<MyListener>();
    app.subscribeEvent("testEvent", listener);
    app.subscribeEvent("numericEvent", listener);
    app.subscribeEvent("mixedEvent", listener);

    std::cout << "发布事件 testEvent...\n";
    app.publishEvent("testEvent", std::launch::async, std::string("来自 testEvent 的消息"));

    std::cout << "发布事件 numericEvent...\n";
    app.publishEvent("numericEvent", std::launch::async, 42, 3.14);

    std::cout << "发布事件 mixedEvent...\n";
    app.publishEvent("mixedEvent", std::launch::async, std::string("Alice"), 30, true);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
