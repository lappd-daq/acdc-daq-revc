#include <iostream>
#include "ACC.h"
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <map>
#include <numeric>


using namespace std;
//This function should be run
//every time new pedestals are
//set using the setConfig function. 
//this function measures the true
//value of the pedestal voltage applied
//by the DACs on-board. It does so by
//taking a lot of software trigger data
//and averaging each sample's measured
//during real data-taking. These pedestals are
//subtracted live during data-taking. 

#define NUM_CH 30
#define NUM_PSEC 5
#define NUM_CH_PER_PSEC 6 
#define NUM_BOARDS 8
#define NUM_SAMPLE 256
#define N_EVENTS 1000
std::atomic<bool> quit(false); //signal flag

void got_signal(int)
{
	quit.store(true);
}

int main()
{
	ACC acc;

	system("mkdir -p Results");
	system("mkdir -p autogenerated_calibrations");
	system("rm ./Results/Data_Config.txt");

	//immediately enter a data collection loop
	//that will save data without pedestals subtracted
	//to get a measurement of the pedestal values. 
	int trigMode = 1; //software trigger for calibration
	unsigned int boardMask = 0xFF;
	int calibMode = 1;
	bool raw = false;
	int retval;
	string datafn;//file to ultimately save avg

	retval = acc.initializeForDataReadout(trigMode, boardMask, calibMode);
	if(retval != 0)
	{
		cout << "Initialization failed!" << endl;
		return 0;
	}
	acc.emptyUsbLine();
	acc.dumpData();
	
	for(int i=0; i<N_EVENTS; i++){
		acc.softwareTrigger();

		retval = acc.listenForAcdcData(trigMode, raw, "Config");
		if (retval!=0)
		{
			cout << "retval " << retval << endl;
			i--;
			acc.dumpData();
			continue;
		}
	}

	system("python3 ./analysis/averaging.py ./Results/Data_Config.txt ./autogenerated_calibrations/");
	return 1;
}
