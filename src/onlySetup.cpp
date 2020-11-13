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

	ACC acc;

	system("mkdir -p Results");
	
	while(flag)
	{
		std::cout << "Please select triggermode: " << std::endl; 
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

		if(triggermode==1)
		{
			break;
		}

		switch(triggermode)
		{
			case 2:
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setInvertMode(invertMode);
				acc.setDetectionMode(detectionMode);
				flag = false;
				break;
			case 3:
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setInvertMode(invertMode);
				acc.setDetectionMode(detectionMode);
				flag = false;
				break;
			case 4:
				goto selfsetup;
			case 5: 
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setInvertMode(invertMode);
				acc.setDetectionMode(detectionMode);
				goto selfsetup;
			case 6:
				std::cout << "Use normal polarity (0) or inverted polarity (1) on ACDC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setInvertMode(invertMode);
				acc.setDetectionMode(detectionMode);
				goto selfsetup;
			case 7:
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setInvertMode(invertMode);
				acc.setDetectionMode(detectionMode);
				flag = false;
				break;
			case 8:
				//
			default:
				std::cout << " Trigger input not found " << std::endl;
				break;
			selfsetup:
				std::cout << "Enable coincidence for self trigger? (0/1)" << std::endl;
				cin >> enableCoin;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				if(enableCoin == 1)
				{
					std::cout << "How many of 30 ch need to be in coincidence to self trigger? (0-30)" << std::endl;
					cin >> ChCoin;
					cin.ignore(numeric_limits<streamsize>::max(),'\n');
					stringstream ss;
					unsigned int hexstr;
					ss << ChCoin;
					ss >> std::hex >> hexstr;
					acc.setChCoin(hexstr);
				}
				std::cout << "Use normal polarity (0) or inverted polarity (1) on ACDC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setEnableCoin(enableCoin);
				acc.setInvertMode(invertMode);
				acc.setDetectionMode(detectionMode);
				flag = false;
				break;
		}
	}
	acc.setTriggerMode(triggermode);
	
	while(true)
	{
		std::cout << "Please select which boards to use from 0x00 to 0xFF (default)" << std::endl;
		
		std::cin >> boardmask;
		cin.ignore(numeric_limits<streamsize>::max(),'\n');

		if(sizeof(boardmask) == 4)
		{
			break;
		}
	}

	while(true)
	{
		std::cout << "Do you want to use the calibration input mode? (0/1)" << std::endl;

		cin >> calibMode;
		cin.ignore(numeric_limits<streamsize>::max(),'\n');

		if(calibMode==0 || calibMode==1)
		{
			break;
		}
	}

	retval = acc.initializeForDataReadout(triggermode, boardmask, calibMode);
	if(retval != 0)
	{
		cout << "Initialization failed!" << endl;
		return 0;
	}

	return 1;
}


