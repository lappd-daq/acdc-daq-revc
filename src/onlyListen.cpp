#include "ACC.h"
#include "Scope.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>
#include <limits>

int main(int argc, char *argv[])
{

	ACC acc;

	int retval;
	int triggermode;
	unsigned int boardmask;
	int calibMode;
	int rawMode;
	bool rawBool;
	int invertMode;
	int detectionMode;
	int ChCoin;
	int enableCoin;
	int eventCounter;
	int eventNumber;
	int failCounter;
	bool flag = true;
	int oscopeMode;

	triggermode = acc.getTriggermode();
	while(true)
	{
		std::cout << "Do you want the raw data? (0/1)" << std::endl;

		cin >> rawMode;
		cin.ignore(numeric_limits<streamsize>::max(),'\n');

		if(rawMode==0)
		{
			rawBool = false;
			break;
		}else if(rawMode==1)
		{
			rawBool = true;
			break;
		}
	}

	flag = true;
	while(flag)
	{
		std::cout << "Do you want to use SAVE mode(0) or OSCOPE(1) mode?" << std::endl;

		cin >> oscopeMode;
		cin.ignore(numeric_limits<streamsize>::max(),'\n');

		if(oscopeMode==0)
		{
			while(true)
			{
				std::cout << "How many events do you want to record?" << std::endl;

				cin >> eventNumber;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				if(eventNumber>0)
				{
					flag = false;
					break;
				}
			}		
		}else if(oscopeMode ==1)
		{
			flag = false;
			eventNumber = 1;
			break;
		}
	}

	eventCounter = 0;
	failCounter = 0;
	if(oscopeMode==0)
	{
		while(eventCounter<eventNumber)
		{
			if(triggermode == 1)
			{
				acc.softwareTrigger();
			}
			retval = acc.listenForAcdcData(triggermode, rawBool, eventCounter, oscopeMode);
			switch(retval)
			{
				case 0:
					std::cout << "Successfully found data and parsed" << std::endl;
					eventCounter++;
					break;
				case 1:
					std::cout << "Successfully found data and but buffer corrupted" << std::endl;
					failCounter++;
					break;
				case 2:
					std::cout << "No data found" << std::endl;
					failCounter++;
					break;
				case 3:
					std::cout << "Sigint failure" << std::endl;
					failCounter++;
					break;
				default:
					std::cout << "Unknown error" << std::endl;
					failCounter++;
					break;
			}
			if(failCounter >= 50)
			{
				break;
			}
		}
	}else if(oscopeMode==1)
	{
		Scope scp;
		int first = 0;

		flag = true;
		while(flag){
			if(triggermode == 1)
			{
				acc.softwareTrigger();
			}
			retval = acc.listenForAcdcData(triggermode, rawBool, eventCounter, oscopeMode);
			switch(retval)
			{
				case 0:
					if(first == 0)
					{
						scp.plot(rawBool);
						first++;
					}
					break;
				case 1:
					std::cout << "Successfully found data and but buffer corrupted" << std::endl;
					break;
				case 2:
					std::cout << "No data found" << std::endl;
					break;
				case 3:
					std::cout << "Sigint failure" << std::endl;
					flag =  false;
					break;
				default:
					std::cout << "Unknown error" << std::endl;
					flag =  false;
					break;
			}
		}

	}

	return 1;
}


