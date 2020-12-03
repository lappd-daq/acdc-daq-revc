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
//retval:  ... 1 and 2 are never returned ... 
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
        data[0] = buffer;
        return -3;      
    }

    //clear the data map prior.
    data.clear();

    //check for fixed words in header
    if(buffer[0] != 0xac9c || buffer[15] != 0xcac9)
    {
        std::cout << "Data buffer header corrupt" << std::endl;
        return -2;
    }

    //Fill data map
    int channel_count = 0;
    for(int i = 16; i < 16 + 256*30; i += 256)
    {
        data.emplace(std::piecewise_construct, std::forward_as_tuple(channel_count), std::forward_as_tuple(buffer.begin()+i, buffer.begin()+(i+256)));
        channel_count++;
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









