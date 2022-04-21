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
#include "EthernetInterface.h"

using namespace std;

void sendWord(unsigned int word, EthernetInterface& eth, bool verbose = false)
{
    unsigned int clearRequest = 0xFFF10000;
    unsigned int lower16 = 0xFFF30000 | (0xFFFF & word);
    unsigned int upper16 = 0xFFF40000 | (0xFFFF & (word >> 16));
    unsigned int setPLL = 0xFFF50000;
    
    eth.send(0x100, clearRequest);
    eth.send(0x100, lower16);
    eth.send(0x100, upper16);
    eth.send(0x100, setPLL);
    eth.send(0x100, clearRequest);

    if(verbose)
    {
	printf("send 0x%08x\n", lower16);
	printf("send 0x%08x\n", upper16);
	printf("send 0x%08x\n", setPLL);
    }
}

int main()
{
    EthernetInterface eth("192.168.133.107", "2007");
    
    // program registers 0 and 1 with approperiate settings for 40 MHz output 
    sendWord(0x55500060, eth);
    //sendWord(0x5557C060, eth); // 125 MHz input
    usleep(2000);    
    sendWord(0x83810001, eth);
    //sendWord(0xFF810081, eth); // 125 MHz input
    usleep(2000);

    // cycle "power down" to force VCO calibration 
    sendWord(0x00001802, eth);
    usleep(2000);
    sendWord(0x00001002, eth);
    usleep(2000);
    sendWord(0x00001802, eth);
    usleep(2000);

    // toggle sync bit to synchronize output clocks
    sendWord(0x0001802, eth);
    usleep(2000);
    sendWord(0x0000802, eth);
    usleep(2000);
    sendWord(0x0001802, eth);
    usleep(2000);

    // read register
//    sendWord(0x0000000e, eth);
//    sendWord(0x00000000, eth);

    // write register contents to EEPROM
    //sendWord(0x0000000f, eth);

    return 0;
}
