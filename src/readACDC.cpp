#include <iostream>
#include <thread>
#include <chrono>
#include "ACC.h"

using namespace std;


//this script has been QC tested by Evan on
//two boards. 2/19/20

//reads ACDC metadata on all detectable
//boards and prints it to stdout
int main() {
	int retval;
	ACC acc;
	retval = acc.createAcdcs(); //detect ACDCs and create ACDC objects
	if(retval == 0)
	{
		cout << "Connected to the ACC, but no ACDCs were detected" << endl;
		return 0;
	}

	acc.softwareTrigger();
	bool waitForAll = true; //require that all ACDC buffers be found for success. 
	retval = acc.readAcdcBuffers(waitForAll); //read ACDC buffer from usb, save and parse in ACDC objects

	//acc.resetAccTrigger();
	//acc.resetAccTrigger();

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
	}
	else
	{
		cout << "ACDCs never returned any data - check acdc alignment, possibly power cycle" << endl;
	}

	return 1;
}