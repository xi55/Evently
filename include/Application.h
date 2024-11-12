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
#include "BaseWork.h"
#include "EventBase.h"
#if defined(__linux__)
    #include <cxxabi.h>
#endif
#include <vector>
#include <shared_mutex>

namespace Evently
{
    class Application
    {
    public:
        template<class T>
        static void getList(std::vector<std::string> &typeNames, std::vector<std::shared_ptr<void>> &list, T &&t)
        {
#if defined(__linux__)
            int status;
            char *type = abi::__cxa_demangle(typeid(t).name(), nullptr, nullptr, &status);
            if (type) 
            {
                typeNames.push_back(std::string(type));
                free(type);
            }
#else
            typeNames.push_back(typeid(t).name());
#endif
            list.push_back(std::make_shared<T>(std::forward<T>(t)));
        }
        
        template<class... Args>
        static void publishEvent(const std::string &evently_name, Args&&... args)
        {
            if (evently_name.empty()) return;
            auto numArgs = sizeof...(args);

            assert(numArgs < 10); // 断言：参数数量不超过10个

            std::unique_lock<std::mutex> lock(ep_lock);
            if (event_pool_high.find(evently_name) != event_pool_high.end() || 
                event_pool.find(evently_name) != event_pool.end()           || 
                event_pool_low.find(evently_name) != event_pool_low.end())
            {
                std::vector<std::shared_ptr<BaseWork>> objList;
                objList.insert(objList.end(), event_pool_high[evently_name].begin(), event_pool_high[evently_name].end());
                objList.insert(objList.end(), event_pool[evently_name].begin(), event_pool[evently_name].end());
                objList.insert(objList.end(), event_pool_low[evently_name].begin(), event_pool_low[evently_name].end());
                auto methodName = std::string(FUNCNAME) + eventName;

                for (auto obj : objList)
                {
                    if (obj)
                    {
                        std::cout << "obj is existence" << std::endl;
                    }
                    else
                    {
                        std::cout << " --------------- " << std::endl;
                    }
                    
                }

            }
        }

    private:
        // 高优先级事件池
        static std::map<std::string, std::vector<std::map<int, std::shared_ptr<BaseWork>>>> event_pool_high;
        // 默认优先级事件池
        static std::map<std::string, std::vector<std::map<int, std::shared_ptr<BaseWork>>>> event_pool;
        // 低优先级事件池
        static std::map<std::string, std::vector<std::map<int, std::shared_ptr<BaseWork>>>> event_pool_low;

        static std::shared_mutex ep_lock; // 事件池锁
    };
}

#endif // APPLICATION_H