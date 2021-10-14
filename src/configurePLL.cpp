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
#include "stdUSB.h"

using namespace std;


unsigned int reverseBits(unsigned int v)     // input bits to be reversed
{
    unsigned int r = v; // r will be reversed bits of v; first get LSB of v
    int s = sizeof(v) * CHAR_BIT - 1; // extra shift needed at end

    for (v >>= 1; v; v >>= 1)
    {   
	r <<= 1;
	r |= v & 1;
	s--;
    }
    r <<= s; // shift when v's highest bits are zero
    return r;
}

void sendWord(unsigned int word, stdUSB* usb, bool verbose = false)
{
    //word = reverseBits(word);

    unsigned int clearRequest = 0xFFF10000;
    unsigned int lower16 = 0xFFF30000 | (0xFFFF & word);
    unsigned int upper16 = 0xFFF40000 | (0xFFFF & (word >> 16));
    unsigned int setPLL = 0xFFF50000;
    
    usb->sendData(clearRequest);
    usb->sendData(lower16);
    usb->sendData(upper16);
    usb->sendData(setPLL);
    usleep(1);
    usb->sendData(clearRequest);

    if(verbose)
    {
	printf("send 0x%08x\n", lower16);
	printf("send 0x%08x\n", upper16);
	printf("send 0x%08x\n", setPLL);
    }
}

int main()
{
    stdUSB* usb = new stdUSB();
    if(!usb->isOpen())
    {
	cout << "Usb was unable to connect to ACC" << endl;
	delete usb;
	return 0;
    }
    
    // program registers 0 and 1 with approperiate settings for 40 MHz output 
    sendWord(0x55500060, usb);
    usleep(2000);    
    sendWord(0x83810001, usb);
    usleep(2000);

    // cycle "power down" to force VCO calibration 
    sendWord(0x00001802, usb);
    usleep(2000);
    sendWord(0x00001002, usb);
    usleep(2000);
    sendWord(0x00001802, usb);
    usleep(2000);

    // toggle sync bit to synchronize output clocks
    sendWord(0x0001802, usb);
    usleep(2000);
    sendWord(0x0000802, usb);
    usleep(2000);
    sendWord(0x0001802, usb);
    usleep(2000);

    // read register
//    sendWord(0x0000000e, usb);
//    sendWord(0x00000000, usb);

    // write register contents to EEPROM
    //sendWord(0x0000000f, usb);

    delete usb;
    
    return 0;
}
