#include <iostream>
#include "CyWebserver.h"
int main() {
    setbuf(stdout, 0);

    CyWeb webserver;
    webserver.Init(9006,5,10);
    webserver.PrepareSql();
    webserver.EpollListen();
    webserver.MainLogic();
    return 0;
}