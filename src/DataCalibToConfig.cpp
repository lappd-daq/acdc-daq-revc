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
vector<string> metadata_keys;

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
					sum[bi][ch+1][smp] += mapdata[evn][bi][ch+1][smp];
				}				
			}
		}
	}

	for(int bi: boards)
	{
		for(int ch=0; ch<NUM_CH; ch++){
			for(int smp=0; smp<NUM_SAMPLE; smp++)
			{
				sum[bi][ch+1][smp] = sum[bi][ch+1][smp]/N_EVENTS;
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

void writePsecData(ofstream& d, vector<int> boardsReadyForRead, map<int, map<int, map<int, vector<double>>>> map_data, map<int, map<int, map<string, unsigned short>>> map_meta)
{
	vector<string> keys;
	keys = metadata_keys;

	string delim = " ";
	for(int evn=0; evn<N_EVENTS; evn++)
	{
		for(int enm=0; enm<NUM_SAMP; enm++)
		{
			d << dec << enm << delim;
			for(int bi: boardsReadyForRead)
			{
				for(int ch=0; ch<NUM_CH; ch++)
				{
					if(enm==0)
					{
						//cout << "Writing board " << bi << " and ch " << ch << ": " << map_data[bi][ch+1][enm] << endl;
					}
					d << map_data[evn][bi][ch+1][enm] << delim;
				}
				if(enm<(int)keys.size())
				{
					d << hex << map_meta[evn][bi][keys[enm]] << delim;

				}else
				{
					d << 0 << delim;
				}
			}
			d << endl;
		}
	}
	d.close();
}

void initializeMetadataKeys()
{
    metadata_keys.push_back("Board");
    //General PSEC info 
    for(int i = 0; i < NUM_PSEC; i++)
    {
        metadata_keys.push_back("feedback_count_"+to_string(i)); //Info 1
        metadata_keys.push_back("feedback_target_count_"+to_string(i)); //Info 2
        metadata_keys.push_back("Vbias_setting_"+to_string(i)); //Info 3
        metadata_keys.push_back("selftrigger_threshold_setting_"+to_string(i)); //Info 4
        metadata_keys.push_back("PROVDD_setting_"+to_string(i)); // Info 5
        metadata_keys.push_back("VCDL_count_lo_"+to_string(i)); //Info 11, later bit(15-0)
        metadata_keys.push_back("VCDL_count_hi_"+to_string(i)); //Info 12, later bit(31-16)
        metadata_keys.push_back("DLLVDD_setting_"+to_string(i)); //Info 13
    }

    //Trigger PSEC settings
    metadata_keys.push_back("trigger_mode"); //Info 6, PSEC1 bit(3-0)
    metadata_keys.push_back("trigger_validation_window_start"); //Info 6, PSEC1 bit(15-4)
    metadata_keys.push_back("trigger_validation_window_length"); //info 6, PSEC2 bit(11-0)
    metadata_keys.push_back("trigger_sma_invert"); // Info 6, PSEC3 bit(1)
    metadata_keys.push_back("trigger_sma_detection_mode"); // Info 6, PSEC3 bit(0)
    metadata_keys.push_back("trigger_acc_invert"); // Info 6, PSEC3 bit(3)
    metadata_keys.push_back("trigger_acc_detection_mode"); // Info 6, PSEC3 bit(2)
    metadata_keys.push_back("trigger_self_sign"); //Info 6, PSEC3 bit(5)
    metadata_keys.push_back("trigger_self_detection_mode"); // Info 6, PSEC3 bit(4)
    metadata_keys.push_back("trigger_self_coin"); // Info 6, PSEC3 bit(10-6)

    metadata_keys.push_back("trigger_selfmask_0"); //Info 7 PSEC0
    metadata_keys.push_back("trigger_selfmask_1"); //Info 7 PSEC1
    metadata_keys.push_back("trigger_selfmask_2"); //Info 7 PSEC2
    metadata_keys.push_back("trigger_selfmask_3"); //Info 7 PSEC3
    metadata_keys.push_back("trigger_selfmask_4"); //Info 7 PSEC4

    metadata_keys.push_back("trigger_self_threshold_0"); //Info 8 PSEC0 bit(11-0)
    metadata_keys.push_back("trigger_self_threshold_1"); //Info 8 PSEC1 bit(11-0)
    metadata_keys.push_back("trigger_self_threshold_2"); //Info 8 PSEC2 bit(11-0)
    metadata_keys.push_back("trigger_self_threshold_3"); //Info 8 PSEC3 bit(11-0)
    metadata_keys.push_back("trigger_self_threshold_4"); //Info 8 PSEC4 bit(11-0)

    //Timestamp data
    metadata_keys.push_back("timestamp_0"); // Info 9 PSEC0 later bit(15-0)
    metadata_keys.push_back("timestamp_1"); // Info 9 PSEC1 later bit(31-16)
    metadata_keys.push_back("timestamp_2"); // Info 9 PSEC2 later bit(47-32)
    metadata_keys.push_back("timestamp_3"); // Info 9 PSEC3 later bit(63-48)

    metadata_keys.push_back("clockcycle_bits"); // Info 9 PSEC0 bit(2-0)

    //Event count
    metadata_keys.push_back("event_count_lo"); //Info 10 PSEC0 later bit(15-0)
    metadata_keys.push_back("event_count_hi"); //Info 10 PSEC1 later bit(31-16)

    for(int ch=0; ch<NUM_CH; ch++)
    {
         metadata_keys.push_back("self_trigger_rate_count_psec_ch"+to_string(ch));
    }
    metadata_keys.push_back("combined_trigger_rate_count");
}

int main(int argc, char *argv[]) //arg 1:in| arg 2: out|  
{
	ifstream ifs(argv[1], ios_base::in);
	string datafn = argv[2];
	datafn += "Reordered_Data.txt";
	ofstream dataofs(datafn.c_str(), ios_base::trunc); //trunc overwrites

	char delim = ' ';
	string lineFromFile; //full line
	string adcCountStr; //string representing adc counts of ped
	double avg; //int for the current channel key

	map<int, map<int, map<int, vector<double>>>> mapdata; //<event, board, channel, data vector>
	map<int, map<int, map<int, vector<double>>>> re_data;
	map<int, map<int, map<string, unsigned short>>> mapmeta;
	vector<int> boardsRead;
	vector<double> temp;

	initializeMetadataKeys();

	for(int evn=0; evn<N_EVENTS; evn++)
	{
		//loop over each line of file
		for(int i=0; i<NUM_SAMP; i++)
		{
			getline(ifs, lineFromFile);
			stringstream line(lineFromFile); //stream of characters delimited

			//todo

			for(int bi: expecBoard)
			{
				try
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
								getline(line, adcCountStr, delim);
								cout << adcCountStr << endl;
								mapmeta[evn][bi][metadata_keys[i]] = stoi(adcCountStr);
								continue;
						}
						getline(line, adcCountStr, delim);
						cout << adcCountStr << endl;
						avg = stod(adcCountStr); //channel key for a while
						mapdata[evn][bi][ch].push_back(avg);
					}
				}catch(string mechanism)
				{
					cout << mechanism << endl;
					return 2;
				}	
			}
		}
	}	

	cout << "Getting boards" << endl;
	boardsRead = getBoards(mapdata);
	cout << "Reordering " << boardsRead.size() << " boards" << endl;
	re_data = reorder(mapdata, mapmeta, boardsRead);

	cout << "Starting write" << endl;
	writePsecData(dataofs, boardsRead, re_data, mapmeta);
	return 1;
}