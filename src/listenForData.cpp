#include "ACC.h"
#include "Scope.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>
#include <limits>
#include <chrono> 
#include <iomanip>
#include <numeric>
#include <ctime>

string getTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%d%m_%H%M%S");
    return ss.str();
}

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
	int setup;
	int threshold;
	string timestamp;
	int py_anal;

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
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge)?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use edge detect (0) or level detect (1)?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Set the threshold of the self trigger in adc counts from 0 to 4095" << std::endl;
				cin >> threshold;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				stringstream ss2;
				unsigned int adcstr;
				ss2 << threshold;
				ss2 >> std::hex >> adcstr;

				acc.setThreshold(adcstr);
				acc.setEnableCoin(enableCoin);
				acc.setInvertMode(invertMode);
				acc.setDetectionMode(detectionMode);
				flag = false;
				break;
		}
	}

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


	retval = acc.initializeForDataReadout(triggermode, boardmask, calibMode);
	if(retval != 0)
	{
		cout << "Initialization failed!" << endl;
		return 0;
	}

	timestamp = getTime();
	eventCounter = 0;
	failCounter = 0;
	int reTime = 500;
	int mult = 1;
	if(oscopeMode==0)
	{	
		while(eventCounter<eventNumber)
		{
			if(triggermode == 1)
			{
				acc.softwareTrigger();
			}
			if(eventCounter>=reTime*mult)
			{
				timestamp = getTime();
				mult++;
			}
			retval = acc.listenForAcdcData(triggermode, rawBool, timestamp, oscopeMode);
			switch(retval)
			{
				case 0:
					std::cout << "Successfully found data and parsed" << std::endl;
					eventCounter++;
					failCounter=0;
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
				std::cout << "Too many failed attempts to read data. Please check everything and try again" << std::endl;
				break;
			}
		}
	}else if(oscopeMode==1)
	{
		int bNum;
		while(true)
		{
			std::cout << "Do you want to use 2 boards for evaluation? If yes (1) only use port 0 and 2. Else choose (0)." << std::endl;
			cin >> bNum;
			cin.ignore(numeric_limits<streamsize>::max(),'\n');

			if(bNum == 0 || bNum == 1)
			{
				break;
			}
		}
		
		Scope scp;
		int first = 0;

		flag = true;
		while(flag){
			if(triggermode == 1)
			{
				acc.softwareTrigger();
			}
			retval = acc.listenForAcdcData(triggermode, rawBool, timestamp, oscopeMode);
			switch(retval)
			{
				case 0:
					if(first == 0)
					{
						scp.plot(rawBool,bNum);
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
	while(true)
	{
		std::cout << "Do you want to use python for the PSEC analysis?(0/1)" << std::endl;
		cin >> py_anal;
		cin.ignore(numeric_limits<streamsize>::max(),'\n');

		if(py_anal==0){
			break;
		}else if(py_anal==1)
		{
			string cmd = "python3 ./analysis/PSEC_analysis.py ./Results/Data_";
			cmd += timestamp;
			cmd += ".txt";
			system(cmd.c_str());
			break;
		}
	}
	return 1;
}


