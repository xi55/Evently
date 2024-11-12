
#ifndef BASEWORK_H
#define BASEWORK_H

#include <thread>
#include <atomic>
#include "nss/nano_signal_slot.hpp"  // 信号槽库，用于替代 Q_SIGNAL 和 Q_SLOT 功能
namespace Evently
{
    class BaseWork {
    public:
        // 移动线程类型枚举
        enum class MoveThreadType {
            MainThread = 0,
            WorkThread,
            NewThread
        };

        // 构造函数和析构函数
        explicit BaseWork();
        virtual ~BaseWork();

        

        // 跨线程移动函数
        void moveToThread(std::thread& thread);

        // 设置运行标志
        void setRunFlag(int value);

        // 获取线程移动类型
        MoveThreadType getMoveType() const;

        // 信号槽机制替代
        Nano::Signal<void()> quit;

        // 虚函数，用于子类实现
        virtual void start() {}

    protected:
        MoveThreadType move_type_;
        std::atomic<int> run_flag_;
    };

}
#endif // BASEWORK_H