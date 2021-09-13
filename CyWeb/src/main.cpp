#include <iostream>
#include "CyWebserver.h"
int main() {
    setbuf(stdout, 0);
    //std::cout << "Hello, World!" << std::endl;
    CyWeb webserver;
    webserver.Init(9006,5,10);
    webserver.EpollListen();
    webserver.MainLogic();
    return 0;
}