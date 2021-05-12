#include "ACC.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>

using namespace std;

int main()
{
	ACC acc;
	stdUSB* usb;

	int retval = acc.whichAcdcsConnected(); 
	if(retval==-1)
	{
		std::cout << "Trying to reset ACDC boards" << std::endl;
		unsigned int command = 0xFFFF0000;
		usb->sendData(command);
		usleep(1000000);
		int retval = acc.whichAcdcsConnected();
		if(retval==-1)
		{
			std::cout << "After ACDC reset no changes, still no boards found" << std::endl;
		}
	}
}
