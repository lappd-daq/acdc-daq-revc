#include "ACDC.h"
#include <bitset>
#include <sstream>
#include <fstream>
#include <chrono> 
#include <iomanip>
#include <numeric>
#include <ctime>

using namespace std;

ACDC::ACDC()
{
	trigMask = 0xFFFFFF;
	convertMaskToChannels();
}



ACDC::~ACDC()
{
	cout << "Calling acdc destructor" << endl;
}

void ACDC::printMetadata(bool verbose)
{
	meta.standardPrint();
	if(verbose) meta.printAllMetadata();
}

void ACDC::setTriggerMask(unsigned int mask)
{
	trigMask = mask;
	convertMaskToChannels();
}

int ACDC::getBoardIndex()
{
	return boardIndex;
}

void ACDC::setBoardIndex(int bi)
{
	boardIndex = bi;
}

unsigned int ACDC::getTriggerMask()
{
	return trigMask;
}

vector<int> ACDC::getMaskedChannels()
{
	return maskedChannels;
}

//reads the value of unsigned int trigMask
//and converts it to a vector of ints 
//corresponding to channels that are masked, 
//i.e. inactive in the trigger logic. 
void ACDC::convertMaskToChannels()
{ 

	//clear the vector
	maskedChannels.clear();
	for(int i = 0; i < NUM_CH; i++)
	{
		if((trigMask & (1 << i)))
		{
			//channel numbering starts at 1
			maskedChannels.push_back(i + 1); 
		}
	}
}


//utility for debugging
void ACDC::writeRawBufferToFile(vector<unsigned short> lastAcdcBuffer)
{
    string fnnn = "raw-acdc-buffer.txt";
    cout << "Printing ACDC buffer to file : " << fnnn << endl;
    ofstream cb(fnnn);
    for(unsigned short k: lastAcdcBuffer)
    {
        printByte(cb, k);
        cb << endl;
    }
    cb.close();
}

void ACDC::printByte(ofstream& ofs, unsigned short val)
{
    ofs << val << ", "; //decimal
    stringstream ss;
    ss << std::hex << val;
    string hexstr(ss.str());
    ofs << hexstr << ", "; //hex
    unsigned n;
    ss >> n;
    bitset<16> b(n);
    ofs << b.to_string(); //binary
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
int ACDC::parseDataFromBuffer(vector<unsigned short> acdc_buffer, bool raw, int bi)
{
	lastAcdcBuffer = acdc_buffer;

	//make sure an acdc buffer has been
	//filled. if not, there is nothing to be done.
	if(lastAcdcBuffer.size() == 0)
	{
		string err_msg = "You tried to parse ACDC data without pulling/setting an ACDC buffer";
		writeErrorLog(err_msg);
		return 2;
	}

	if((peds.size() == 0 || conv.size() == 0) && !raw)
	{
		string err_msg = "Found no pedestal or LUT conversion data but was told to parse data. Please check the ACC class for an initialization of this calibration data";
		writeErrorLog(err_msg);
		return 2;
	}

	//clear the data map prior.
	data.clear();

	//word that indicates the data is
	//about to start for each psec chip.
	unsigned short startword = 0xF005; 
	unsigned short endword = 0xBA11; //just for safety, uses the 256 sample rule. 
	int channelCount = 0; //iterates every 256 samples and when a startword is found 
	int sampleCount = 0; //for checking if we hit 256 samples. 
	double sampleValue = 0.0; //temporary holder for the sample in ADC-counts/mV
	bool dataFlag = false; //if we are currently on psec-data bytes. 
	vector<double> waveform; //the current channel's data as a vector of doubles. 
	//a for loop through the whole buffer and save states
	for(unsigned short byte: lastAcdcBuffer)
	{
		if(byte == startword && !dataFlag)
		{
			//re-initialize for a new PSEC chip
			dataFlag = true;
			sampleCount = 0;
			channelCount++;
			continue;
		}
		
		if(byte == endword && dataFlag)
		{
			dataFlag = false;
			//push the last waveform to data.
			if(waveform.size() != NUM_SAMP)
			{
				//got a corrupt data buffer, throw event away
				string err_msg = "Got a corrupt buffer with ";
				err_msg += to_string(waveform.size());
				err_msg += " number of samples on a chip after saving ";
				err_msg += to_string(channelCount);
				err_msg += " channels (1)";
				writeErrorLog(err_msg);
				data.clear();

				return 1;
			} 
			//cout << "Channel " << channelCount << " gets " << waveform.size() << " samples " << endl;
			data[channelCount] = waveform;
			waveform.clear();
			//dont iterate channel, itl happen at
			//the startword if statement. 
			continue;
		}

		if(dataFlag)
		{
			//here is where we assume every
			//channel to have NUM_SAMP samples. 
			if(sampleCount == NUM_SAMP)
			{
				sampleCount = 0;
				if(waveform.size() != NUM_SAMP)
				{
					//got a corrupt data buffer, throw event away
					string err_msg = "Got a corrupt buffer with ";
					err_msg += to_string(waveform.size());
					err_msg += " number of samples on a chip after saving ";
					err_msg += to_string(channelCount);
					err_msg += " channels (2)";
					writeErrorLog(err_msg);
					data.clear();
					return 1;
				} 
				//cout << "Channel " << channelCount << " gets " << waveform.size() << " samples " << endl;
				data[channelCount] = waveform;
				waveform.clear();
				channelCount++;
			}
			if(channelCount > NUM_CH)
			{
				//we are done here.
				channelCount--; //reset to = NUM_CH 
				break; //could also be continue.
			}

			//---these lines fill a waveform vector
			//---that will be inserted into the data map
			sampleValue = (double)byte; //adc counts

			if(!raw)
			{	
				//apply a pedestal subtraction
				sampleValue = sampleValue - peds[bi][channelCount-1]; //adc counts
				//apply a linearity corrected mV conversion
				//sampleValue = sampleValue*conv[channelCount][sampleCount]; //mV
			}
			
			//save in the vector. vector is saved in the data map when
			//the channel count is iterated. 
			waveform.push_back(sampleValue); 
			sampleCount++;
			continue;
		}


	}

	//depending on the type of corrupt buffer, the above loop
	//will happily record fewer than NUM_CH. 
	if(channelCount != NUM_CH)
	{
		string err_msg = "Got a corrupt buffer with ";
		err_msg += to_string(channelCount);
		err_msg += " number of channels";
		writeErrorLog(err_msg);
		data.clear();
		return 1;
	}

	bool corruptBuffer;
	corruptBuffer = meta.parseBuffer(acdc_buffer);

	//map_meta = meta.getMetadata();

	if(corruptBuffer)
	{
		return 3;
	}	
	return 0;

}


//writes data from the presently stored event
// to file assuming file has header already
void ACDC::writeDataForOscope(ofstream& d)
{
	string delim = " ";

	for(int row = 1; row<=NUM_SAMP; row++)
	{
		d << row << delim;
		for(int column=1; column<=NUM_CH; column++)
		{
			d << data[column][row-1] << delim; 
		}
		d << endl;
	}
}


//writes data from the presently stored event
// to file assuming file has header already
void ACDC::writeRawDataToFile(vector<unsigned short> buffer, ofstream& d)
{
    for(unsigned short k: buffer)
    {
        printByte(d, k);
        d << endl;
    }
    d.close();
	return;
}


//reads pedestals to file in a new
//file format relative to the old software. 
//<channel> <sample 1> <sample 2> ...
//<channel> ...
//samples in ADC counts. 
void ACDC::readPedsFromFile(ifstream& ifs)
{
	char delim = ' '; //in between ADC counts
	map<int, map<int, double>> tempPeds;//temporary holder for the new pedestal map

	//temporary variables for line parsing
	string lineFromFile; //full line
	string adcCountStr; //string representing adc counts of ped
	double avg; //int for the current channel key
	int bi =0;
	int ch=0;

	//loop over each line of file
	while(getline(ifs, lineFromFile))
	{
		stringstream line(lineFromFile); //stream of characters delimited

		//loop over each sample index
		while(getline(line, adcCountStr, delim))
		{

			avg = stoi(adcCountStr); //channel key for a while
			tempPeds[bi][ch] = avg;

			ch++;
		}

		bi++;
	}

	//call public member of this class to set the pedestal map
	setPeds(tempPeds);
	return;
}


//reads LUT conversions to file in a new
//file format relative to the old software. 
//<channel> <sample 1> <sample 2> ...
//<channel> ...
//samples in ADC counts. 
void ACDC::readConvsFromFile(ifstream& ifs)
{
	char delim = ' '; //in between ADC counts
	map<int, vector<double>> tempConvs;//temporary holder for the new conversion map

	//temporary variables for line parsing
	string lineFromFile; //full line
	string adcCountStr; //string representing adc counts of conv
	int ch; //int for the current channel key
	vector<double> tempWav; //conv wav temporary 
	bool isChannel; //is this character the channel key

	//loop over each line of file
	while(getline(ifs, lineFromFile))
	{
		stringstream line(lineFromFile); //stream of characters delimited
		isChannel = true; //first character is the channel key
		tempWav.clear(); //fresh vector
		//loop over each sample index
		while(getline(line, adcCountStr, delim))
		{
			if(isChannel)
			{
				ch = stoi(adcCountStr); //channel key for a while
				isChannel = false;
				continue; //go to next delimited word (start of adcCounts)
			}
			tempWav.push_back(stod(adcCountStr)); //conversions in adcCounts
		}

		//now set this vector to the appropriate conv map element
		tempConvs.insert(pair<int, vector<double>>(ch, tempWav));
	}

	//call public member of this class to set the conversion map
	setConv(tempConvs);
	return;
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









