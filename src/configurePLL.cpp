#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <map>
#include <numeric>


using namespace std;

int main()
{
    stdUSB* usb = new stdUSB();
	if(!usb->isOpen())
	{
		cout << "Usb was unable to connect to ACC" << endl;
		delete usb;
		return 0;
	}
	
	unsigned int l16 = 0x1234;
    unsigned int u16 = 0x1234;
	
	unsigned int clearRequest = 0xFFF10000;
    unsigned int lower16 = 0xFFF30000 | l16;
    unsigned int upper16 = 0xFFF40000 | u16;
    unsigned int setPLL = 0xFFF50000;
    
    usb->sendData(clearRequest);
    usb->sendData(lower16);
    usb->sendData(upper16);
    usb->sendData(setPLL);
    usb->sendData(clearRequest);
    
    printf("send 0x%08x\n", lower16);
    printf("send 0x%08x\n", upper16);
    printf("send 0x%08x\n", setPLL);

    return 1;
}