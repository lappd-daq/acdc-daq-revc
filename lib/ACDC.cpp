#include "ACDC.h"
#include <bitset>
#include <sstream>
#include <fstream>
#include <chrono> 
#include <iomanip>
#include <numeric>
#include <ctime>

using namespace std;

ACDC::ACDC(){}

ACDC::~ACDC(){}

int ACDC::getBoardIndex()
{
	return boardIndex;
}

void ACDC::setBoardIndex(int bi)
{
	boardIndex = bi;
}

//looks at the last ACDC buffer and organizes
//all of the data into a data map. The boolean
//argument toggles whether you want to subtract
//pedestals and convert ADC-counts to mV live
//or keep the data in units of raw ADC counts. 
//retval: 
//2: other error
//1: corrupt buffer 
//0: all good
int ACDC::parseDataFromBuffer(vector<unsigned short> buffer)
{
	//Catch empty buffers
	if(buffer.size() == 0)
	{
		std::cout << "You tried to parse ACDC data without pulling/setting an ACDC buffer" << std::endl;
		return -1;
	}

	if(buffer.size() == 16)
	{
		pps = buffer;
		data.clear();
		return -3;	
	}

	//clear the data map prior.
	data.clear();

	//Helpers
	int DistanceFromZero;
	int channel_count=0;

	//Indicator words for the start/end of the data
	const unsigned short startword = 0xF005; 
	unsigned short endword = 0xBA11;
	unsigned short endoffile = 0x4321;

	//Empty vector with positions of aboves startword
	vector<int> start_indices; 

	//Find the startwords and write them to the vector
	vector<unsigned short>::iterator bit;
	for(bit = buffer.begin(); bit != buffer.end(); ++bit)
	{
		if(*bit == startword)
		{
			DistanceFromZero= std::distance(buffer.begin(), bit);
			start_indices.push_back(DistanceFromZero);
		}
	}

	//Filter in cases where one of the start words is found in the metadata 
	if(start_indices.size()>NUM_PSEC)
	{
		for(int k=0; k<(int)start_indices.size()-1; k++)
		{
		    if(start_indices[k+1]-start_indices[k]>6*256+14)
		    {
				//nothing
		    }else
		    {
				start_indices.erase(start_indices.begin()+(k+1));
				k--;
		    }
		}
	}

	//Last case emergency stop if metadata is still not quite right
	if(start_indices.size() != NUM_PSEC)
	{
		string fnnn = "acdc-corrupt-psec-buffer.txt";
		cout << "Printing to file : " << fnnn << endl;
		ofstream cb(fnnn);
		for(unsigned short k: buffer)
		{
		    cb << hex << k << endl;
		}
		return -2;
	}

	//Fill data map
	for(int i: start_indices)
	{
		//Write the first word after the startword
		bit = buffer.begin() + (i+1);

		//As long as the endword isn't reached copy metadata words into a vector and add to map
		vector<unsigned short> InfoWord;
		while(*bit != endword && *bit != endoffile)
		{
			InfoWord.push_back((unsigned short)*bit);
			if(InfoWord.size()==NUM_SAMP)
			{
				data.insert(pair<int, vector<unsigned short>>(channel_count, InfoWord));
				InfoWord.clear();
				channel_count++;
			}
			++bit;
		}	
	}

	if(data.size()!=NUM_CH)
	{
		cout << "error 1: Not 30 channels" << endl;
	}

	for(int i=0; i<NUM_CH; i++)
	{
		if(data[i].size()!=NUM_SAMP)
		{
			cout << "error 2: not 256 samples in channel " << i << endl;
		}
	}

	return 0;
}


void ACDC::writeErrorLog(string errorMsg)
{
    string err = "errorlog.txt";
    cout << "------------------------------------------------------------" << endl;
    cout << errorMsg << endl;
    cout << "------------------------------------------------------------" << endl;
    ofstream os_err(err, ios_base::app);
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%m-%d-%Y %X");
    os_err << "------------------------------------------------------------" << endl;
    os_err << ss.str() << endl;
    os_err << errorMsg << endl;
    os_err << "------------------------------------------------------------" << endl;
    os_err.close();
}









