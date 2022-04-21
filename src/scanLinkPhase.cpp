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
#include <vector>
#include "EthernetInterface.h"

using namespace std;


int main()
{
    EthernetInterface eth("192.168.133.107", "2007");

    usleep(10000);

    eth.send(0x0052, 0xffff);

    for(int iChan = 0; iChan < 8; ++iChan)
    {
	std::vector<int> errors;
      
	for(int iOffset = 0; iOffset < 24; ++iOffset)
	{
	    eth.send(0x0054, 0x0000);
	    eth.send(0x0055, 0x0000 + iChan);
	    eth.send(0x0056, 0x0000);

	    //sendData(usb, 0x00510000 + (iDelay << 2));
	    //sendData(usb, 0x00500000);

	    usleep(100);

	    eth.send(0x0100, 0xfff60000);

	    usleep(1000);

	    eth.send(0x0100, 0xfff60001);

	    usleep(100);

	    eth.send(0x0053, 0x0000);

	    usleep(1000);

            uint64_t decode_errors = eth.recieve(0x1120 + iChan);

	    printf("%3d, %5d, ",iOffset, iChan);
            printf("%10d, ", uint32_t(decode_errors));

            uint64_t prbs_errors = eth.recieve(0x1110 + iChan);

//	    printf("%3d, %5d, ",iOffset, iChan);
            printf("%10d\n", uint32_t(prbs_errors));

	    errors.push_back(uint32_t(decode_errors));
	}

	int stop = 0;
	int length = 0;
	int length_best = 0;
	for(int i = 0; i < int(2*errors.size()); ++i)
	{
	    int imod = i % errors.size();
	    if(errors[imod] == 0)
	    {
		++length;
	    }
	    else
	    {
		if(length >= length_best)
		{
		    stop = imod;
		    length_best = length;
		}
		length = 0;
//                if(i > int(errors.size())) break;
	    }
	}
	int phaseSetting = (stop - length_best/2)%errors.size();
	printf("Phase setting selected: %d\n\n", phaseSetting);
	eth.send(0x0054, 0x0000);
	eth.send(0x0055, 0x0000 + iChan);
	for(int i = 0; i < phaseSetting; ++i)
	{
	    eth.send(0x0056, 0x0000);	    
	}
    }

    eth.send(0x100, 0xfff60000);
    
    return 0;
}
