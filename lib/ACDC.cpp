#include "ACDC.h"
#include <bitset>
#include <sstream>
#include <fstream>

using namespace std;

ACDC::ACDC()
{
	trigMask = 0xFFFFFF;
	convertMaskToChannels();
}



ACDC::~ACDC()
{
	cout << "Calling acdc detructor" << endl;
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
		cout << "You tried to parse ACDC data without pulling/setting an ACDC buffer" << endl;
		return 2;
	}

	if((pedestal.size() == 0 || conv.size() == 0) && !raw)
	{
		cout << "Found no pedestal or LUT conversion data but was told to parse data." << endl;
		cout << "Please check the ACC class for an initialization of this calibration data" << endl;
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
				cout << "Got a corrupt buffer with " << waveform.size() << " number of samples on a chip after saving " << channelCount << " channels (1)" << endl;
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
					cout << "Got a corrupt buffer with " << waveform.size() << " number of samples on a chip after saving " << channelCount << " channels (2)" << endl;
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
				sampleValue = sampleValue*conv[channelCount][sampleCount]; //mV
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
		cout << "Got a corrupt buffer with " << channelCount << " number of channels " << endl;
		data.clear();
		return 1;
	}

	bool corruptBuffer;
	corruptBuffer = meta.parseBuffer(acdc_buffer);

	if(corruptBuffer)
	{
		return 3;
	}	
	return 0;

}

void ACDC::readPED(vector<unsigned short> acdc_buffer)
{
	unsigned short startword = 0xF005; 
	unsigned short endword = 0xBA11;  
	int channelCount = 1; //iterates every 256 samples and when a startword is found 
	int sampleCount = 0; //for checking if we hit 256 samples. 
	double sampleValue = 0.0; //temporary holder for the sample in ADC-counts/mV
	bool dataFlag = false; //if we are currently on psec-data bytes. 
	vector<double> waveform; //the current channel's data as a vector of doubles. 
	int n_sample = 64;
	double sum, num, mean, delta;
	vector<double> pedestal;
	double limit = 200;

	for(unsigned short byte: acdc_buffer)
	{
		if(byte == startword && !dataFlag)
		{
			//re-initialize for a new PSEC chip
			dataFlag = true;
			sampleCount = 0;
			continue;
		}

		if(byte == endword && dataFlag)
		{
			dataFlag = false;
			waveform.clear();
			continue;
		}

		if(dataFlag)
		{
			//here is where we assume every
			//channel to have NUM_SAMP samples. 
			if(sampleCount == NUM_SAMP-1)
			{
				sum = 0;
				num = 0;
				// use waveform to make pedestal
				for(int i=0; i< n_sample; i++)
				{
					delta = waveform[i] - waveform[i+1];
					if(abs(delta) > limit)
					{
						break;
					}
					sum += waveform[i];
					num++;
				}
				mean = sum/num;
				//cout << "ch " << channelCount << " mean: " << mean << endl;
				pedestal.push_back(mean);

				sampleCount = 0;
				waveform.clear();
				channelCount++;
				continue;
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

			//save in the vector. vector is saved in the data map when
			//the channel count is iterated. 
			waveform.push_back(sampleValue); 
			sampleCount++;
			continue;
		}
	}
	setPed(pedestal);
	if(pedestal.size() != 30)
	{
		cout << "Couldn't get 30 pedestal values"  << endl;
		for(double val: pedestal)
		{
			cout << val << ", "; //decimal
		}
		cout << endl;
	}
	pedestal.clear();
}


//writes data from the presently stored event
// to file assuming file has header already
void ACDC::writeDataToFile(ofstream& d, ofstream& m, int oscopeOnOff)
{
	string delim = " ";

	if(oscopeOnOff == 0)
	{
		//metadata part is simple and contained
		//in that class. 
		meta.writeMetadataToFile(m, delim);

		int evno = meta.getEventNumber();

		vector<double> waveform;//temporary storage of wave data
		int channel; //temporary storage of channel
		map<int, vector<double>>::iterator it;
		for(it = data.begin(); it != data.end(); ++it)
		{
			waveform = it->second;
			channel = it->first;
			//first three columns are event, board, channel
			d << evno << delim << boardIndex << delim << channel;
			//loop the vector and print values
			for(double s: waveform)
			{
				d << delim << s;
			}
			d << endl;
		}

		cout << "done" << endl;
		return;
	}else if(oscopeOnOff==1)
	{
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



//writes pedestals to file in a new
//file format relative to the old software. 
//<channel> <sample 1> <sample 2> ...
//<channel> ...
//samples in ADC counts. 
void ACDC::writePedsToFile(ofstream& ofs)
{/*
	string delim = " ";

	map<int, vector<double>>::iterator mit;
	vector<double>::iterator vit;
	vector<double> tempwav; //ped data
	int ch; //channel
	for(mit = peds.begin(); mit != peds.end(); ++mit)
	{
		tempwav = mit->second;
		ch = mit->first;

		
		ofs << ch << delim;//print channel to file with a delim
		for(vit = tempwav.begin(); vit != tempwav.end(); ++vit)
		{
			ofs << *vit << delim; //print ped value for that sample 
		}
		ofs << endl;
	}

	return;
	*/
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


//writes LUT conversions to file in a new
//file format relative to the old software. 
//<channel> <sample 1> <sample 2> ...
//<channel> ...
//samples in ADC counts. 
void ACDC::writeConvsToFile(ofstream& ofs)
{
	string delim = " ";

	map<int, vector<double>>::iterator mit;
	vector<double>::iterator vit;
	vector<double> tempwav; //conv data
	int ch; //channel
	for(mit = conv.begin(); mit != conv.end(); ++mit)
	{
		tempwav = mit->second;
		ch = mit->first;

		
		ofs << ch << delim;//print channel to file with a delim
		for(vit = tempwav.begin(); vit != tempwav.end(); ++vit)
		{
			ofs << *vit << delim; //print conv value for that sample 
		}
		ofs << endl;
	}

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


//takes a datafile and loads the data member with evno's data. 
//this is used for minor analysis codes independent of some
//ACC. It is somewhat inefficient. 
map<int, vector<double>> ACDC::readDataFromFile(vector<string> fileLines, int evno)
{
	map<int, vector<double>> returnData;
	string word;
	int ch; //channel curent
	int ev; //event number current
	int bo; //board index present in file
	char delim = ' ';

	//Inefficient loop through 
	//a vector to find the right event and board. 
	for(string line: fileLines)
	{
      stringstream ssline(line); //the current line in the file
      getline(ssline, word, delim); //first word is the event
      ev = stoi(word);
      getline(ssline, word, delim);
      bo = stoi(word); //board is 2nd word
      getline(ssline, word, delim);
      ch = stoi(word); //channel is third word;
      //we are on an acceptable line
      if(ev == evno && bo == boardIndex)
      {
          vector<double> tempwav;
          //get all of the adc counts in the channel
          while(getline(ssline, word, delim))
          {
              tempwav.push_back(stod(word));
          }
          //error check
          if(tempwav.size() != NUM_SAMP)
          {
              cout << "In reading data, found an event that has not the expected number of samples: " << tempwav.size() << endl;
          }
          returnData.insert(pair<int, vector<double>>(ch, tempwav));
		   }
	}

	//error checking
	if(returnData.size() != NUM_CH)
	{
		cout << "In reading data, found an event with different number of channels than expected: " << returnData.size() << endl;
	}

	setData(returnData);
	return returnData;

	

}








