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




std::atomic<bool> quit(false); //signal flag

void got_signal(int)
{
	quit.store(true);
}

map<int, map<int, double>> calculatePedMap(map<int, map<int, vector<double>>> runsum)
{
	map<int, map<int, double>> tempdata;
	map<int, map<int, double>> avgdata;
	double sum,mean;

	for(int bi=0; bi<8; bi++)
	{
		for(int ch=0; ch<30; ch++)
		{
			sum = accumulate(runsum[bi][ch].begin(), runsum[bi][ch].end() , 0);
			tempdata[bi][ch] = sum;
		}
	}
	for(int bi=0; bi<8; bi++)
	{
		for(int ch=0; ch<30; ch++)
		{
			mean = tempdata[bi][ch]/(50*256);
			avgdata[bi][ch] = mean;
		}
	}
	return avgdata;
}

int main() {

	string datafn;//file to ultimately save avg

	//immediately enter a data collection loop
	//that will save data without pedestals subtracted
	//to get a measurement of the pedestal values. 
	int trigMode = 1; //software trigger for calibration
	unsigned int boardMask = 0xFF;
	int calibMode = 1;
	int nevents = 50; //old software read 50 times for ped calibration. 
	double sum;
	int retval;
	
	ACC* acc = new ACC();

	retval = acc->initializeForDataReadout(trigMode,boardMask,calibMode);
	if(retval != 0)
	{
		cout << "Initialization failed!" << endl;
		return 0;
	}

	map<int, map<int, vector<double>>> tempdata;
	map<int, map<int, vector<double>>> runsum;
	map<int, map<int, double>> pedMap;
	for(int i=0; i<nevents; i++){
		acc->softwareTrigger();

		retval = acc->listenForAcdcData(trigMode,true,1,0);
		if (retval!=0)
		{
			cout << "retval " << retval << endl;
			i--;
			continue;
		}
		tempdata = acc->returnPedData();
		for(int bi=0; bi<tempdata.size(); bi++)
		{
			for(int ch=0; ch<tempdata[bi].size(); ch++)
			{
				sum = accumulate(tempdata[bi][ch].begin(), tempdata[bi][ch].end() , 0);
				runsum[bi][ch].push_back(sum);
			}
		}
	}

	pedMap = calculatePedMap(runsum);


	//open files that will hold the most-recent PED data. 
	datafn = CALIBRATION_DIRECTORY;
	string mkdata = "mkdir -p ";
	mkdata += CALIBRATION_DIRECTORY;
	system(mkdata.c_str());
	datafn += PED_TAG; 
	datafn += ".txt";
	cout << datafn << endl;
	ofstream dataofs(datafn.c_str(), ios_base::trunc); //trunc overwrites
	for(int bi=0; bi<8; bi++)
	{
		for(int ch=0; ch<30; ch++)
		{
			dataofs << pedMap[bi][ch] << " ";
		}
		dataofs << endl;
	}

	dataofs.close();

	return 1;
}