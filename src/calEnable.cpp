#include "ACC.h"
#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[])
{
	int onoff; //on or off, 1 or 0
	//board mask argument needs to be in binary string (11010110)
	unsigned int boardmask = 0xFF; //0xFF means all 8 boards are switched to cal mode
	//channel argument needs to be in hex WITH the 0x. 
	//channels are ganged in pairs due to hardware reasons
	//with the calibration switch. So 0x0001 means channels 1 and 2
	//are enabled. 
	unsigned int channelmask = 0xFFFF; //0xFFFF means all 30 channels are cal enabled

	if(argc > 4 || argc == 1)
	{
		cout << "Usage: ./bin/calEnable <0/1 on off> [board number (binary 11111111) (= all)] [channel pairs (hex mask)(= 0xFFFF)] " << endl;
		return 0;
	}
	else
	{
		onoff = atoi(argv[1]);
		if(argc >= 3)
		{
			char *temp;
			boardmask = strtoul(argv[2], &temp, 2); //the 2 means you can enter as 11110101. 
		}
		if(argc == 4)
		{
			char *temp;
			channelmask = strtoul(argv[3], &temp, 0); //needs to 0x to be interpreted as hex. 
		}
		cout << "Setting cal to " << onoff << " on boards " << std::hex << boardmask << " and channels " << std::hex << channelmask << endl;

		//create an ACC object.
		ACC acc;
		acc.createAcdcs(); //detect ACDCs and create ACDC objects

		acc.toggleCal(onoff, boardmask, channelmask);
		
	}
	

	return 0;
}