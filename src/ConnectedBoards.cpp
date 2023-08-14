#include "ACC_ETH.h"
#include "ACC_USB.h"
#include <iostream>
#include <cstring> 

using namespace std;

int main(int argc, char *argv[])
{   
    if (argc <= 1)
    {
        std::cout << "Please enter an option for the connection as well:" << std::endl;
        std::cout << "./ConnectedBoards [USB or ETH]" << std::endl;
        return 0;
    }

    ACC_ETH* acc_eth = nullptr;
    ACC_USB* acc_usb = nullptr;

    if(strcmp(argv[1], "USB") == 0)
    {
        acc_usb = new ACC_USB();
    }else if(strcmp(argv[1], "ETH") == 0)
    {
        acc_eth = new ACC_ETH("192.168.133.1","0");
    }else
    {
        std::cout << "Please enter a valid connection option" << std::endl;
        return 0;
    }

    if(acc_usb)
    {
        acc_usb->versionCheck();
        delete acc_usb;
    }else if(acc_eth)
    {
        acc_eth->VersionCheck();
        delete acc_eth;
    }

    return 1;
}
