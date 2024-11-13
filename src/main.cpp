#include <iostream>
#include <any>
#include "Evently.h"
class MyClass 
{
public:
    MyClass() 
    {
        REGISTER_METHOD(MyClass, printMessage);
        REGISTER_METHOD(MyClass, add);
        REGISTER_FIELD(MyClass, number);
        REGISTER_FIELD(MyClass, name);
    }

    void printMessage(const std::string& msg) {
        std::cout << "Message: " << msg << std::endl;
    }

    int add(int a, int b) {
        return a + b;
    }

    int number = 42;
    std::string name = "MyClass";
};

REGISTER_CLASS(MyClass)

int main() 
{
    Evently::ReflectionRegistry& registry = Evently::ReflectionRegistry::getInstance();
    MyClass obj;

    registry.invokeMethod("MyClass", "printMessage", &obj, {std::string("Hello, Reflection!")});
    int result = std::any_cast<int>(registry.invokeMethod("MyClass", "add", &obj, {3, 5}));
    std::cout << "Addition result: " << result << std::endl;

    int number = std::any_cast<int>(registry.getValues("MyClass", "number", &obj));
    std::string name = std::any_cast<std::string>(registry.getValues("MyClass", "name", &obj));
    std::cout << "Number: " << number << ", Name: " << name << std::endl;

    return 0;
}
