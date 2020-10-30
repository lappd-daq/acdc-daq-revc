#include "ACC.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>

int main(int argc, char *argv[]){

	if(argc<4){
		std::cout << "Not enough input arguments! Please use ./readCalibModeData triggermode boardmask(default:all) calibmode-on/off" << std::endl;
		return 0;
	}

	int retval;
	ACC acc;

	stringstream ss;
	unsigned int cmd;
	ss << argv[2];
	ss >> std::hex >> cmd;

	retval = acc.initializeForDataReadout(atoi(argv[1]), cmd, atoi(argv[3]));
	if(retval != 0){
		cout << "Initialization failed!" << endl;
	}

	acc.softwareTrigger();

	retval = acc.listenForAcdcData(atoi(argv[1]), true);
	if(retval == 0){
		std::cout << "Successfully read and wrote data" << std::endl;
	}else{
		std::cout << "Read failed" << std::endl;
	}
}
