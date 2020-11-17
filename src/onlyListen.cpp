#include "ACC.h"
#include "Scope.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>
#include <limits>

ACC acc;

int main()
{
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

	system("mkdir -p Results");

	std::cout << "Please select triggermode: (Does not set anything just reminds the software) " << std::endl; 
	std::cout << "(0) Off" << std::endl;
	std::cout << "(1) Software trigger" << std::endl;
	std::cout << "(2) SMA trigger ACC" << std::endl;
	std::cout << "(3) SMA trigger ACDC" << std::endl;
	std::cout << "(4) Self-trigger" << std::endl;
	std::cout << "(5) Self-trigger with validation ACC" << std::endl;
	std::cout << "(6) Self-trigger with validation ACDC" << std::endl;
	std::cout << "(7) SMA ACC with SMA ACDC" << std::endl;
	std::cout << "(8) SMA ACDC with SMA ACC" << std::endl;
	
	std::cin >> triggermode;
	cin.ignore(numeric_limits<streamsize>::max(),'\n');


	acc.createAcdcs();

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


