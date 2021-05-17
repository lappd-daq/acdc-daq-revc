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

using namespace std;

ACC acc;

int retval;
//Triggermode
int triggermode;
int invertMode;
int detectionMode;
int ChCoin;
int enableCoin;
int threshold;
stringstream ss3;
stringstream ss4;
int validationWindow;
int validationStart;
unsigned int valstr;
unsigned int valstr2;
unsigned int psec_chip;
unsigned int psec_channel;
std::vector<unsigned int> vec_psec_chip;
std::vector<unsigned int> vec_psec_channel;

void setTrigger()
{
	bool flag = true;
	int run=0;
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
		std::cout << "Set it to: ";

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
				std::cout << "Enter: ";
				cin >> detectionMode; 	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode; 		cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 2);
				acc.setDetectionMode(detectionMode, 2);

				flag = false;
				break;
			case 3:
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA?" << std::endl;
				std::cout << "Enter: ";
				cin >> detectionMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode;		cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 3);
				acc.setDetectionMode(detectionMode, 3);

				flag = false;
				break;
			case 4:
				goto selfsetup;
			case 5: 
				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA validation?" << std::endl;
				std::cout << "Enter: ";
				cin >> detectionMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA validation?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "How long should the validation window start be delayed from 0 to 819us in 25ns steps?" << std::endl;
				std::cout << "Enter in us: ";
				cin >> validationStart;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss3 << std::hex << (int)(validationWindow*40);
				valstr = std::stoul(ss3.str(),nullptr,16);

				std::cout << "How long should the validation window be from 0 to 819us in 25ns steps?" << std::endl;
				std::cout << "Enter in us: ";
				cin >> validationWindow;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss4 << std::hex << (int)(validationWindow*40);
				valstr2 = std::stoul(ss4.str(),nullptr,16);

				acc.setValidationStart(valstr);
				acc.setValidationWindow(valstr2);
				acc.setSign(invertMode, 2);
				acc.setDetectionMode(detectionMode, 2);

				goto selfsetup;
			case 6:
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA validation?" << std::endl;
				std::cout << "Enter: ";
				cin >> detectionMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA validation?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "How long should the validation window start be delayed from 0 to 819us in 25ns steps?" << std::endl;
				std::cout << "Enter in us: ";
				cin >> validationStart;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss3 << std::hex << (int)(validationWindow*40);
				valstr = std::stoul(ss3.str(),nullptr,16);

				std::cout << "How long should the validation window be from 0 to 819us in 25ns steps?" << std::endl;
				std::cout << "Enter in us: ";
				cin >> validationWindow;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss4 << std::hex << (int)(validationWindow*40);
				valstr2 = std::stoul(ss4.str(),nullptr,16);

				acc.setValidationStart(valstr);
				acc.setValidationWindow(valstr2);
				acc.setSign(invertMode, 3);
				acc.setDetectionMode(detectionMode, 3);

				goto selfsetup;
			case 7:
				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA?" << std::endl;
				std::cout << "Enter: ";
				cin >> detectionMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 2);
				acc.setDetectionMode(detectionMode, 2);

				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA validation?" << std::endl;
				std::cout << "Enter: ";
				cin >> detectionMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA validation?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 3);
				acc.setDetectionMode(detectionMode, 3);

				std::cout << "How long should the validation window start be delayed from 0 to 819us in 25ns steps?" << std::endl;
				std::cout << "Enter in us: ";
				cin >> validationStart;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss3 << std::hex << (int)(validationWindow*40);
				valstr = std::stoul(ss3.str(),nullptr,16);

				std::cout << "How long should the validation window be from 0 to 819us in 25ns steps?" << std::endl;
				std::cout << "Enter in us: ";
				cin >> validationWindow;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss4 << std::hex << (int)(validationWindow*40);
				valstr2 = std::stoul(ss4.str(),nullptr,16);

				acc.setValidationStart(valstr);
				acc.setValidationWindow(valstr2);

				flag = false;

				break;
			case 8:
				std::cout << "Use edge detect (0) or level detect (1) on ACDC SMA?" << std::endl;
				std::cout << "Enter: ";
				cin >> detectionMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACDC SMA?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 3);
				acc.setDetectionMode(detectionMode, 3);

				std::cout << "Use edge detect (0) or level detect (1) on ACC SMA validation?" << std::endl;
				std::cout << "Enter: ";
				cin >> detectionMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) on ACC SMA validation?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				acc.setSign(invertMode, 2);
				acc.setDetectionMode(detectionMode, 2);

				std::cout << "How long should the validation window start be delayed from 0 to 819us in 25ns steps?" << std::endl;
				std::cout << "Enter in us: ";
				cin >> validationStart;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss3 << std::hex << (int)(validationWindow*40);
				valstr = std::stoul(ss3.str(),nullptr,16);

				std::cout << "How long should the validation window be from 0 to 819us in 25ns steps?" << std::endl;
				std::cout << "Enter in us: ";
				cin >> validationWindow;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				ss4 << std::hex << (int)(validationWindow*40);
				valstr2 = std::stoul(ss4.str(),nullptr,16);

				acc.setValidationStart(valstr);
				acc.setValidationWindow(valstr2);

				flag = false;

				break;
			default:
				std::cout << " Trigger input not found " << std::endl;
				break;
			selfsetup:
				std::cout << "Use edge detect (0) or level detect (1) for self trigger?" << std::endl;
				std::cout << "Enter: ";
				cin >> detectionMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Use normal polarity (0, high level or rising edge) or inverted polarity (1, low level or falling edge) for selftrigger?" << std::endl;
				std::cout << "Enter: ";
				cin >> invertMode;	cin.ignore(numeric_limits<streamsize>::max(),'\n');

				std::cout << "Enable coincidence for self trigger? (0/1)" << std::endl;
				std::cout << "Enter: ";
				cin >> enableCoin;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				if(enableCoin == 1)
				{
					std::cout << "How many of 30 ch need to be in coincidence to self trigger? (0-30)" << std::endl;
					std::cout << "Enter: ";
					cin >> ChCoin;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
					stringstream ss;
					unsigned int hexstr;
					ss << ChCoin;
					ss >> std::hex >> hexstr;
					acc.setNumChCoin(hexstr);
				}

				std::cout << "Set the threshold of the self trigger in adc counts from 0 to 4095" << std::endl;
				std::cout << "Enter: ";
				cin >> threshold;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
				stringstream ss2;
				unsigned int adcstr;
				ss2 << threshold;
				ss2 >> std::hex >> adcstr;

				while(run<5)
				{
					std::cout << "Do you want to enable channels on Psec chip " << run << "? (0/1)" << std::endl;
					std::cout << "Enter: ";
					cin >> psec_chip;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
					if(psec_chip==1)
					{
						std::cout << "What channels of psec chip" << run << "do you want to enable? (From 0x00 to 0x3F, each bit representing a channel)" << std::endl;
						std::cout << "Enter: ";
						cin >> psec_channel;	cin.ignore(numeric_limits<streamsize>::max(),'\n');
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
}

int main()
{
	//General	
	unsigned int boardmask=0xFF;
	int calibMode=0;
	bool rawBool=true;
	int choice;
	bool flag=true;

	while(flag)
	{
		std::cout << "What do you want to do?" << std::endl; 
		std::cout << "(0) Check connected boards" << std::endl;
		std::cout << "(1) Get firmware versions" << std::endl;
		std::cout << "(2) Setup boards" << std::endl;
		std::cout << "(3) Exit" << std::endl;
		std::cout << "Enter: ";

		std::cin >> choice; cin.ignore(numeric_limits<streamsize>::max(),'\n');
		std::cout << endl;
		switch(choice)
		{
			case 0:
				acc.connectedBoards();
				break;
			case 1:
				acc.versionCheck();
				break;
			case 2:
				std::cout << "--------------------Starting setup--------------------" << std::endl;
				setTrigger();
				retval = acc.initializeForDataReadout(triggermode, boardmask, calibMode);
				if(retval != 0)
				{
					cout << "Initialization failed!" << endl;
					return 0;
				}
				acc.emptyUsbLine();
				acc.dumpData();

				std::cout << "------------------------------------------------------" << std::endl;
				std::cout << "---Setup done. Please use Crtl+C to end acquisition---" << std::endl;
				while(true)
				{
					if(triggermode==0)
					{
						acc.softwareTrigger();
					}
					retval = acc.listenForAcdcData(triggermode, rawBool, "invalidTime"); 
					if(retval==3)
					{
						acc.dumpData();
						flag = false;
						break;
					}else if(retval==0)
					{
						//nothing
					}else
					{
						acc.dumpData();
					}
				}
				break;
			case 3:
				flag = false;
				break;
			default:
				break;
		}
		std::cout << std::endl;
	}
	
	return 1;
}

