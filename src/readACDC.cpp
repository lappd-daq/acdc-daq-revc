#include <iostream>
#include <thread>
#include <chrono>
#include "ACC.h"

using namespace std;

//reads ACDC metadata on all detectable
//boards and prints it to stdout
int main() {

	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects
	cout << "out of create acdcds" << endl;
	acc.softwareTrigger(); //trigger all boards. 
	cout << "out of software trig" << endl;
	acc.readAcdcBuffers(); //read ACDC buffer from usb, save and parse in ACDC objects
	cout << "out of read buffers" << endl;

	bool verbose = false;
	acc.printAcdcInfo(verbose);
	//this_thread::sleep_for(chrono::milliseconds(5));
	//acc.softwareTrigger();
	//acc.readAcdcBuffers();

	return 1;
}