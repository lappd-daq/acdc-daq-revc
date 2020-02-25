#include <iostream>
#include "ACC.h"

using namespace std;
//This function should be run
//every time new pedestals are
//set using the setConfig function. 
//this function measures the true
//value of the pedestal voltage applied
//by the DACs on-board. It does so by
//taking a lot of software trigger data
//and averaging each sample's measured
//voltage over all events. The data is then
//stored in an ascii text file and re-loaded
//during real data-taking. These pedestals are
//subtracted live during data-taking. 






int main() {

	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects

	acc.softwareTrigger();
	bool waitForAll = true; //require that all ACDC buffers be found for success. 
	int retval = acc.readAcdcBuffers(waitForAll); //read ACDC buffer from usb, save and parse in ACDC objects

	acc.resetAccTrigger();
	acc.resetAccTrigger();

	//only print if readAcdcBuffers
	//successfully retreived and parsed
	//data. 
	if(retval == 0)
	{
		bool verbose = false;
		acc.printAcdcInfo(verbose);
	}
	else if(retval == 1)
	{
		bool verbose = false;
		acc.printAcdcInfo(verbose);
		cout << "***Found data but got a corrupt buffer.***" << endl;
		return 0;
	}
	else
	{
		cout << "ACDCs never returned any data - check acdc alignment, possibly power cycle" << endl;
		return 0;
	}


	//now initiate the pedestal measurement


	return 1;
}