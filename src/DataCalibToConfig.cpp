#include <iostream>
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
//during real data-taking. These pedestals are
//subtracted live during data-taking. 

#define NUM_CH 30
#define NUM_PSEC 5
#define NUM_CH_PER_PSEC 6 
#define NUM_BOARDS 8
#define NUM_SAMPLE 256
#define N_EVENTS 100
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

vector<double> reorder_internal(vector<double> temp_vec, unsigned short cycle)
{
	vector<double> re_vec;

	int clockcycle = cycle*32;

    for(int i=0; i<NUM_SAMPLE; i++)
    {
        if(i<(NUM_SAMPLE-clockcycle))
        {
            re_vec.push_back(temp_vec[i+clockcycle]);
        }
        else
        {
            re_vec.push_back(temp_vec[i-(NUM_SAMPLE-clockcycle)]);
        }
    }

    return re_vec;
}

map<int, map<int, map<int, vector<double>>>> reorder(map<int, map<int, map<int, vector<double>>>> mapdata, map<int, map<int, map<string, unsigned short>>> mapmeta, vector<int> boardsRead)
{
	vector<double> temp_vec;
	unsigned short cycle;
	vector<double> re_vec;
	map<int, map<int, map<int, vector<double>>>> re_data;
	for(int evn=0; evn<N_EVENTS; evn++)
	{
		for(int bi: boardsRead)
		{
			cycle = mapmeta[evn][bi]["clockcycle_bits"];
			for(int ch=0; ch<NUM_CH; ch++){
				temp_vec = mapdata[evn][bi][ch+1];			
				re_vec = reorder_internal(temp_vec,cycle);
				re_data[evn][bi][ch+1] = re_vec;
			}
		}	
	}
	return re_data;
}

int main(int argc, char *argv[])
{
	ifstream ifs(argv[1], ios_base::in);

	char delim = ' ';
	string lineFromFile; //full line
	string adcCountStr; //string representing adc counts of ped
	double avg; //int for the current channel key

	string datafn;//file to ultimately save avg
	map<int, map<int, map<int, vector<double>>>> mapdata; //<event, board, channel, data vector>
	map<int, map<int, map<int, vector<double>>>> re_data;
	map<int, map<int, map<string, unsigned short>>> mapmeta;
	map<int, map<int, vector<double>>> avg_data; //<board, channel, data vector>
	vector<int> boardsRead;
	vector<double> temp;

	vector<int> connectedBoards;
	for(int arg=0; arg<argc-3; arg++)
	{
		connectedBoards.push_back(atoi(argv[arg+3]));
	}

	for(int evn=0; evn<N_EVENTS; evn++)
	{
		//loop over each line of file
		for(int i=0; i<NUM_SAMP; i++)
		{
			getline(ifs, lineFromFile);
			stringstream line(lineFromFile); //stream of characters delimited
			for(int bi: connectedBoards)
			{
				//loop over each sample index
				for(int ch=0; ch<NUM_CH+2; ch++)
				{
					if(ch==0)
					{
						getline(line, adcCountStr, delim);
						continue;
					}
					if(ch==NUM_CH+1)
					{
						if(i==26)
						{
							getline(line, adcCountStr, delim);
							cout << adcCountStr << endl;
							mapmeta[evn][bi]["clockcycle_bits"] = stoi(adcCountStr);
						}
						break;
					}
					getline(line, adcCountStr, delim);
					avg = stoi(adcCountStr); //channel key for a while
					if(evn==0 && i==0)
					{
						cout << avg << ", ";				
					}
					mapdata[evn][bi][ch].push_back(avg);
				}
				if(evn==0 && i==0){
					cout << endl;
				}

			}
		}
	}	

	cout << "Getting boards" << endl;
	boardsRead = getBoards(mapdata);
	cout << "Reordering " << endl;
	re_data = reorder(mapdata, mapmeta, boardsRead);
	cout << "Getting average" << endl;
	avg_data = getAverage(re_data, boardsRead);

	cout << "Starting write" << endl;
	//open files that will hold the most-recent PED data.
	for(int bi: boardsRead)
	{
		datafn = argv[2];
		datafn += "PEDS_ACDC"; 
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