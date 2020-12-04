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
#define N_EVENTS 5
std::atomic<bool> quit(false); //signal flag

void got_signal(int)
{
	quit.store(true);
}

vector<int> getBoards(map<int, map<int, map<int, vector<double>>>> mapdata)
{
	int evn = 0;
	vector<int> boards;

	for(auto const& element : mapdata[evn]) {
		boards.push_back(element.first);
	}

	return boards;
}

map<int, map<int, vector<double>>> getAverage(map<int, map<int, map<int, vector<double>>>> mapdata, vector<int> boards)
{
	cout << "Average start" << endl;
	map<int, map<int, vector<double>>> sum;
	vector<double> insert;

	for(int bi: boards)
	{
		for(int ch=0; ch<NUM_CH; ch++)
		{
			for(int smp=0; smp<NUM_SAMPLE; smp++)
			{
				insert.push_back(0);
			}
			sum[bi][ch] = insert;				
		}
	}

	for(int evn=0; evn<N_EVENTS; evn++)
	{
		for(int bi: boards)
		{
			for(int ch=0; ch<NUM_CH; ch++){
				for(int smp=0; smp<NUM_SAMPLE; smp++)
				{
					sum[bi][ch][smp] += mapdata[evn][bi][ch+1][smp];
				}				
			}
		}
	}

	for(int bi: boards)
	{
		for(int ch=0; ch<NUM_CH; ch++){
			for(int smp=0; smp<NUM_SAMPLE; smp++)
			{
				sum[bi][ch][smp] = sum[bi][ch][smp]/N_EVENTS;
			}				
		}
	}
	
	cout << "Average done" << endl;
	return sum;
}

int main()
{
	ACC acc;

	//immediately enter a data collection loop
	//that will save data without pedestals subtracted
	//to get a measurement of the pedestal values. 
	int trigMode = 1; //software trigger for calibration
	unsigned int boardMask = 0xFF;
	int calibMode = 1;
	int oscope = 0;
	bool raw = true;
	int retval;
	string datafn;//file to ultimately save avg
	map<int, map<int, map<int, vector<double>>>> mapdata; //<event, board, channel, data vector>
	map<int, map<int, vector<double>>> avg_data; //<board, channel, data vector>
	vector<int> boardsRead;

	retval = acc.initializeForDataReadout(trigMode, boardMask, calibMode);
	if(retval != 0)
	{
		cout << "Initialization failed!" << endl;
		return 0;
	}

	for(int i=0; i<N_EVENTS; i++){
		acc.softwareTrigger();

		acc.listenForAcdcData(trigMode, raw, "Config", oscope);
		if (retval!=0)
		{
			cout << "retval " << retval << endl;
			i--;
			continue;
		}
		mapdata[i] = acc.returnData();
	}

	cout << "Getting boards" << endl;
	boardsRead = getBoards(mapdata);
	cout << "Getting average" << endl;
	avg_data = getAverage(mapdata, boardsRead);

	cout << "Starting write" << endl;
	//open files that will hold the most-recent PED data.
	for(int bi: boardsRead)
	{
		datafn = CALIBRATION_DIRECTORY;
		string mkdata = "mkdir -p ";
		mkdata += CALIBRATION_DIRECTORY;
		system(mkdata.c_str());
		datafn += PED_TAG; 
		datafn += "_board";

		datafn += to_string(bi);
		datafn += ".txt";
		ofstream dataofs(datafn.c_str(), ios_base::trunc); //trunc overwrites
		cout << "Generating " << datafn << " ..." << endl;

		string delim = " ";
		for(int enm=0; enm<NUM_SAMPLE; enm++)
		{
			for(int ch=0; ch<NUM_CH; ch++)
			{
				dataofs << avg_data[bi][ch][enm] << delim;
			}
			dataofs << endl;
		}
		dataofs.close();
	}
	return 1;
}