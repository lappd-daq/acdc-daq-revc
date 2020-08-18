#include "ACC.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		cout << "Usage: ./bin/reset <mode>" << endl;
		cout << "Where mode is either: usb, acdc, hard" << endl;
		return 0;
	}
	else
	{
		string arg = string(argv[1]);
		if(arg == "hard" || arg == "acdc" || arg == "usb")
		{
			//create an ACC object.
			ACC acc;
			acc.createAcdcs(); //detect ACDCs and create ACDC objects
			if(arg == "hard")
			{
				acc.hardReset();
				sleep(5);
				acc.usbWakeup();
			}
			else if(arg == "usb")
			{
				acc.usbWakeup();
			}
			else if(arg == "acdc")
			{
				acc.resetACDCs();
			}
		}
		else
		{
			cout << "Usage: ./bin/reset <mode>" << endl;
			cout << "Where mode is either: usb, acdc, hard" << endl;
			return 0;
		}
		
	}


	return 0;
}
