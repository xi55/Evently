#include "BaseWork.h"


BaseWork::BaseWork() : run_flag_(true), move_type_(MoveThreadType::WorkThread) {}

BaseWork::~BaseWork() {
    run_flag_ = false;
    quit.fire();  // 发射 quit 信号，表示对象将被销毁
}

void BaseWork::moveToThread(std::thread& thread) {
    if (thread.joinable()) {
        // 连接线程开始和结束时的操作
        // 这里简化为调用 start() 和释放资源
        thread = std::thread([this]() {
            start();
        });
    }
}

void BaseWork::setRunFlag(int value) {
    run_flag_ = value;
}

BaseWork::MoveThreadType BaseWork::getMoveType() const {
    return move_type_;
}
