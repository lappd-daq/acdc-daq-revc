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

//    if(!usb->sendData(0x00000000))
//    {
//	printf("Send Error!!!\n");
//    }
//
//    usleep(10000);

    if(!usb->sendData(0x0052ffff))
    {
	printf("Send Error!!!\n");
    }

    for(int i = 6; i < 8; ++i)
    {
	for(int iOffset = 0; iOffset < 24; ++iOffset)
	{
	    if(!usb->sendData(0x00540000))
	    {
		printf("Send Error!!!\n");
	    }

	    if(!usb->sendData(0x00550000 + i))
	    {
		printf("Send Error!!!\n");
	    }

	    if(!usb->sendData(0x00560000))
	    {
		printf("Send Error!!!\n");
	    }

	    for(int iDelay = 0x0; iDelay < 1/*0x20*/; iDelay += 1)
	    {

		if(!usb->sendData(0x00510000 + (iDelay << 2)))
		{
		    printf("Send Error!!!\n");
		}

		if(!usb->sendData(0x00500000))
		{
		    printf("Send Error!!!\n");
		}

		usleep(100);

		if(!usb->sendData(0xfff60000))
		{
		    printf("Send Error!!!\n");
		}

		usleep(10000);

		if(!usb->sendData(0xfff60001))
		{
		    printf("Send Error!!!\n");
		}

		usleep(100);

		if(!usb->sendData(0x00530000))
		{
		    printf("Send Error!!!\n");
		}

		usleep(1000);

		if(!usb->sendData(0x00230000 + i))
		{
		    printf("Send Error!!!\n");
		}

		std::vector<unsigned short> buffer = usb->safeReadData(256);

		printf("%3d, %4d, %5d, ",iOffset, iDelay, i);
		for(auto& datum : buffer)
		{
		    printf("%10d, ", datum);
		}
	
		if(!usb->sendData(0x00230000 + i + 16))
		{
		    printf("Send Error!!!\n");
		}

		buffer = usb->safeReadData(256);

		for(auto& datum : buffer)
		{
		    printf("%10d\n", datum);
		}
	    }
	}
    }

    if(!usb->sendData(0xfff60000))
    {
	printf("Send Error!!!\n");
    }
    
    return 0;
}
