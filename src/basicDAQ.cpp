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

void usbWakeup(stdUSB* usb)
{
	unsigned int command = 0x00200000;
	bool usbcheck=usb->sendData(command);
	//if(usbcheck==false){writeErrorLog("Send Error");}
}


bool emptyUsbLine(stdUSB* usb)
{
    int send_counter = 0; //number of usb sends
    int max_sends = 10; //arbitrary. 
    unsigned int command = 0x00200000; // a refreshing command
    vector<unsigned short> tempbuff;
    while(true)
    {
	usb->safeReadData(100000);
	bool usbcheck=usb->sendData(command);
	//if(usbcheck==false){writeErrorLog("Send Error");}
	send_counter++;
	tempbuff = usb->safeReadData(100000);

	//if it is exactly an ACC buffer size, success. 
	if(tempbuff.size() == 32)
	{
	    return true;
	}
	if(send_counter > max_sends)
	{
	    string err_msg = "Something wrong with USB line, waking it up. got ";
	    err_msg += to_string(tempbuff.size());
	    err_msg += " words";
	    //writeErrorLog(err_msg);
	    usbWakeup(usb);
	    tempbuff = usb->safeReadData(100000);
	    if(tempbuff.size() == 32){
		return true;
	    }else{
		//writeErrorLog("Usb still sleeping. Problem is not fixed.");
		return false;
	    }
	}
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

    bool clearCheck = emptyUsbLine(usb);
    if(clearCheck==false)
    {
	printf("Failed to clear USB line on constructor!");
    }

    // reset ACC
//    if(!usb->sendData(0x00000000))
//    {
//    	printf("Send Error!!!\n");
//    }
//    
//    usleep(100000);
//    
    // reset ACDC
    if(!usb->sendData(0xffff0000))
    {
    	printf("Send Error!!!\n");
    }
    
    usleep(10000);

    // reset ACC data input FIFOs
    if(!usb->sendData(0x000100ff))
    {
	printf("Send Error!!!\n");
    }

    usleep(10000);

    // disable all calib switches
    if(!usb->sendData(0xffc00000))
    {
	printf("Send Error!!!\n");
    }    

    // disable user calib input
    if(!usb->sendData(0xffc10000))
    {
	printf("Send Error!!!\n");
    }

    // set trig mode 2 in ACC (HARDWAR TRIG)
    if(!usb->sendData(0x00300FF2))
    {
	printf("Send Error!!!\n");
    }

    // set ACDC high speed data outputs to IDLE 
    if(!usb->sendData(0xFFF60000))
    {
	printf("Send Error!!!\n");
    }

    // set ACDC trigger mode to OFF
    if(!usb->sendData(0xFFB00000))
    {
	printf("Send Error!!!\n");
    }

    // send manchester training pulse (backpressure and trig)
    if(!usb->sendData(0x00600000))
    {
	printf("Send Error!!!\n");
    }

    // Enable ACDC trig mode EXT
    if(!usb->sendData(0xFFB00001))
    {
	printf("Send Error!!!\n");
    }

    // Enable ACDC data output mode 3 (DATA)
    if(!usb->sendData(0xFFF60003))
    {
	printf("Send Error!!!\n");
    }

    usleep(100000);
    
    // reset ACC link error counters 
    if(!usb->sendData(0x00530000))
    {
	printf("Send Error!!!\n");
    }

    usleep(1000);

    // send software trigger
    if(!usb->sendData(0x00100000))
    {
	printf("Send Error!!!\n");
    }

    usleep(1);
    
    for(int i = 0; i < 8; ++i)
    {
	// Read FIFO occupancy
	if(!usb->sendData(0x00230000 + 48 + i))
	{
	    printf("Send Error!!!\n");
	}

	std::vector<unsigned short> buffer = usb->safeReadData(10);

	if(buffer.size() > 0)
	{
	    printf("ACDC %1d: %10d\n", i, buffer[0]);

	    if(buffer[0] >= 7680)
	    {
		// Read data FIFO
		if(!usb->sendData(0x00220000+i))
		{
		    printf("Send Error!!!\n");
		}

		std::vector<unsigned short> buffer = usb->safeReadData(20000);
		unsigned int size = buffer.size();
		if(size <= 0) break;
		printf("Words read: %d\n", size);
		printf("Header start: %12x\n", buffer[0]);
		printf("Event count:  %12d\n", buffer [1]);
		printf("sys time: %16llu\n", (uint64_t(buffer[2]) << 48) | (uint64_t(buffer[3]) << 32) | (uint64_t(buffer[4]) << 16) | (uint64_t(buffer[5]) << 0));
		printf("wr time (s):  %12llu\n", (uint64_t(buffer[6]) << 16) | (uint64_t(buffer[7]) << 0));
		printf("wr time (us): %12llu\n", (uint64_t(buffer[8]) << 16) | (uint64_t(buffer[9]) << 0));
		printf("Header end:   %12x\n", buffer[15]);
		printf("Channel : ");
		for(int i = 0; i < 5; ++i) printf("%6d%6d%6d%6d%6d%6d ", 0 + i*6, 1 + i*6, 2 + i*6, 3 + i*6, 4 + i*6, 5 + i*6);
		printf("\n");
		for(int i = 0; i < 256; ++i)
		{
		    printf("%7d : ", i);
		    for(int k = 0; k < 5; ++k)
		    {
			printf("%6x%6x%6x%6x%6x%6x ",
			       buffer[16 + i + 0*256 + k*(256*6)],
			       buffer[16 + i + 1*256 + k*(256*6)],
			       buffer[16 + i + 2*256 + k*(256*6)],
			       buffer[16 + i + 3*256 + k*(256*6)],
			       buffer[16 + i + 4*256 + k*(256*6)],
			       buffer[16 + i + 5*256 + k*(256*6)]);
		    }
		    printf("\n");
		}
	    }
	}

    }


    return 0;
}
