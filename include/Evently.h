#ifndef EVENTLY_H
#define EVENTLY_H

#include "Reflection.h"

namespace Evently
{
    // 自动注册类名
    #define REGISTER_CLASS(className) \
        struct className##AutoRegister { \
            className##AutoRegister() { \
                Evently::ReflectionRegistry::getInstance().registerClassName<className>(#className); \
            } \
        }; \
        static className##AutoRegister global_##className##AutoRegister;

    // 自动注册成员函数
    #define REGISTER_METHOD(className, methodName) \
        struct className##_##methodName##AutoRegister { \
            className##_##methodName##AutoRegister() { \
                Evently::ReflectionRegistry::getInstance().registerMethod<className>(#className, #methodName, &className::methodName); \
            } \
        }; \
        static className##_##methodName##AutoRegister global_##className##_##methodName##AutoRegister;

    // 自动注册成员变量（带类名）
    #define REGISTER_FIELD_WITH_CLASSNAME(className, fieldName) \
        struct className##_##fieldName##AutoRegister { \
            className##_##fieldName##AutoRegister() { \
                Evently::ReflectionRegistry::getInstance().registerField<className>(#className, #fieldName, &className::fieldName); \
            } \
        }; \
        static className##_##fieldName##AutoRegister global_##className##_##fieldName##AutoRegister;

    // 自动注册成员变量（自动使用类名）
    #define REGISTER_FIELD(className, fieldName) \
        struct className##_##fieldName##AutoRegister { \
            className##_##fieldName##AutoRegister() { \
                Evently::ReflectionRegistry::getInstance().registerField(#fieldName, &className::fieldName); \
            } \
        }; \
        static className##_##fieldName##AutoRegister global_##className##_##fieldName##AutoRegister;

}

#endif // EVENTLY_H
