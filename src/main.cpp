#include <iostream>
#include <thread>
#include "BaseWork.h"
#include "nss/nano_signal_slot.hpp"
#include <functional>

void onQuit() {
    std::cout << "Work has been quit.\n";
}

int main() 
{
    BaseWork work;

    work.quit.connect<&onQuit>();

    std::thread workerThread;
    work.moveToThread(workerThread);

    work.setRunFlag(0);

    if (workerThread.joinable()) {
        workerThread.join();
    }

    return 0;
}