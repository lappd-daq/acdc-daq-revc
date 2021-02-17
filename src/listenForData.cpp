#include "ACC.h"
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
#include <vector>

string getTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%d%m_%H%M%S");
    return ss.str();
}

void writeErrorLog(string errorMsg)
{
    std::string err = "errorlog.txt";
    std::cout << "------------------------------------------------------------" << std::endl;
   	std::cout << errorMsg << endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    ofstream os_err(err, ios_base::app);
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%m-%d-%Y %X");
    os_err << "------------------------------------------------------------" << std::endl;
    os_err << ss.str() << endl;
    os_err << errorMsg << endl;
    os_err << "------------------------------------------------------------" << std::endl;
    os_err.close();
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
	stringstream ss3;
	unsigned int valstr;
	string timestamp;
	int run=0;
	int validationWindow;
	unsigned int psec_chip;
	unsigned int psec_channel;
	std::vector<unsigned int> vec_psec_chip;
	std::vector<unsigned int> vec_psec_channel;



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
		std::cout << "(7) SMA ACC with validation SMA ACDC" << std::endl;
		std::cout << "(8) SMA ACDC with validation SMA ACC" << std::endl;
		std::cout << "(9) Pulse-per-second trigger mode" << std::endl;

		std::cin >> triggermode;
		cin.ignore(numeric_limits<streamsize>::max(),'\n');

		if(triggermode==1 || triggermode==9)
		{
			break;
		}

		switch(triggermode)
		{
			case 2:
				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 2);
				acc.setDetectionMode(detectionMode, 2);
				flag = false;
				break;
			case 3:
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 3);
				acc.setDetectionMode(detectionMode, 3);
				flag = false;
				break;
			case 4:
				goto selfsetup;
			case 5: 
				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA validation?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA validation?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "How long should the validation window be from 0 to 25us?" << std::endl;
				cin >> validationWindow;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss3 << validationWindow;
				ss3 >> std::hex >> valstr;

				acc.setValidationWindow(valstr);
				acc.setSign(invertMode, 2);
				acc.setDetectionMode(detectionMode, 2);
				goto selfsetup;
			case 6:
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA validation?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA validation?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "How long should the validation window be from 0 to 25us?" << std::endl;
				cin >> validationWindow;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss3 << validationWindow;
				ss3 >> std::hex >> valstr;

				acc.setValidationWindow(valstr);
				acc.setSign(invertMode, 3);
				acc.setDetectionMode(detectionMode, 3);
				goto selfsetup;
			case 7:
				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 2);
				acc.setDetectionMode(detectionMode, 2);

				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA validation?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA validation?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 3);
				acc.setDetectionMode(detectionMode, 3);

				std::cout << "How long should the validation window be from 0 to 25us?" << std::endl;
				cin >> validationWindow;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss3 << validationWindow;
				ss3 >> std::hex >> valstr;

				acc.setValidationWindow(valstr);
				flag = false;
				break;
			case 8:
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 3);
				acc.setDetectionMode(detectionMode, 3);

				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA validation?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA validation?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 2);
				acc.setDetectionMode(detectionMode, 2);

				std::cout << "How long should the validation window be from 0 to 25us?" << std::endl;
				cin >> validationWindow;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss3 << validationWindow;
				ss3 >> std::hex >> valstr;

				acc.setValidationWindow(valstr);
				flag = false;
				break;
			default:
				std::cout << " Trigger input not found " << std::endl;
				break;
			selfsetup:
				std::cout << "Use edge detect (0) or level detect (1) for self trigger?" << std::endl;
				cin >> detectionMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) for selftrigger?" << std::endl;
				cin >> invertMode;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
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
					acc.setNumChCoin(hexstr);
				}
				std::cout << "Set the threshold of the self trigger in adc counts from 0 to 4095" << std::endl;
				cin >> threshold;
				cin.ignore(numeric_limits<streamsize>::max(),'\n');
				stringstream ss2;
				unsigned int adcstr;
				ss2 << threshold;
				ss2 >> std::hex >> adcstr;

				while(run<5)
				{
					std::cout << "Do you want to enable channels on Psec chip " << run << "? (0/1)" << std::endl;
					cin >> psec_chip;
					cin.ignore(numeric_limits<streamsize>::max(),'\n');
					if(psec_chip==1)
					{
						std::cout << "What channels of psec chip" << run << "do you want to enable? (From 0x00 to 0x3F, each bit representing a channel)" << std::endl;
						cin >> psec_channel;
						cin.ignore(numeric_limits<streamsize>::max(),'\n');
					}else
					{
						run++;
						continue;
					}

					vec_psec_chip.push_back(run);
					vec_psec_channel.push_back(psec_chip);
					run++;
				}

				acc.setThreshold(adcstr);
				acc.setEnableCoin(enableCoin);
				acc.setSign(invertMode, 4);
				acc.setDetectionMode(detectionMode, 4);
				acc.setPsecChipMask(vec_psec_chip);
				acc.setPsecChannelMask(vec_psec_channel);
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
		std::cout << "Do you want to use raw mode(0/1)? Yes means you get a file with 7795 words in hex, No will give the usual data format." << std::endl;

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


	retval = acc.initializeForDataReadout(triggermode, boardmask, calibMode);
	if(retval != 0)
	{
		cout << "Initialization failed!" << endl;
		return 0;
	}
	acc.emptyUsbLine();
	acc.dumpData();
	timestamp = getTime();
	eventCounter = 0;
	failCounter = 0;
	int reTime = 500;
	int mult = 1;
	auto t0 = std::chrono::high_resolution_clock::now();

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
		retval = acc.listenForAcdcData(triggermode, rawBool, timestamp);
		switch(retval)
		{
			case 0:
				//std::cout << "Successfully found data and parsed" << std::endl;
				eventCounter++;
				failCounter=0;
				break;
			case 1:
				writeErrorLog("Successfully found data and but buffer corrupted");
				acc.dumpData();
				failCounter++;
				break;
			case 2:
				writeErrorLog("No data found");
				acc.dumpData();
				failCounter++;
				break;
			case 3:
				writeErrorLog("Sigint failure");
				acc.dumpData();
				failCounter=50;
				break;
			default:
				writeErrorLog("Unknown error");
				failCounter++;
				break;
		}
		if(failCounter >= 50)
		{
			std::cout << "Too many failed attempts to read data. Please check everything and try again" << std::endl;
			break;
		}
	}
	
	auto t1 = std::chrono::high_resolution_clock::now();
	auto dt = 1.e-9*std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
	cout << "It took "<< dt <<" second(s)."<< endl;
	return 1;
}


