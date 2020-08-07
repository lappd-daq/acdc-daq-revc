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
void ACDC::writeRawBufferToFile()
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
//0: success
//1: zero size of ACDC buffer
//2: no peds or conversion data.
//3: no end word in buffer
//4: Not the same psec start indices as end indices
//5: Not the same psec end indices as metadata end indices
//6: Not NUM_PSEC worth of chip data.
//7: Some chip returned the wrong number of samples, needs to be NUM_CH_PER_CHIP*NUM_SAMP
int ACDC::parseDataFromBuffer(vector<unsigned short> b, int eventNumber)
{

	lastAcdcBuffer = b; //store the raw buffer for safe keeping

	//make sure an acdc buffer has been
	//filled. if not, there is nothing to be done.
	if(lastAcdcBuffer.size() == 0)
	{
		cout << "The last ACDC buffer has zero size! See ACDC:parseDataFromBuffer" << endl;
		return 1;
	}

	if(peds.size() == 0 || conv.size() == 0)
	{
		cout << "Found no pedestal or LUT conversion data but was told to parse data." << endl;
		cout << "Please check the ACC class for an initialization of this calibration data" << endl;
		return 2;
	}

	//definitions of words. must match firmware data format.
	unsigned short endword = 0x4321;
	unsigned short psec_frame_start = 0xF005;
	unsigned short psec_postamble_start = 0xBA11;
	unsigned short metadata_end_word = 0xFACE;


	vector<int> psec_start_indices; //start of psec data blocks.
	vector<int> psec_end_indices; //end of psec data blocks
	vector<int> metadata_end_indices; //end of metadata blocks
	int total_end_index = 0; //end of total buffer, defines where trigger data lives


	//find indices of flags in the data buffer.
	for(int i = 0; i < (int)b.size(); i++)
	{
		if(b.at(i) == psec_frame_start) {psec_start_indices.push_back(i);}
		if(b.at(i) == psec_postamble_start && b.at(i-1) != psec_postamble_start) {psec_end_indices.push_back(i);}
		if(b.at(i) == metadata_end_word) {metadata_end_indices.push_back(i);}
		if(b.at(i) == endword) {total_end_index = i;}
	}

	//-----------error management-------------//
	//another way would be to collect all errors and return
	//a vector of int error codes.
	if(total_end_index == 0)
	{
		cout << "No end word found in acdc buffer" << endl;
		return 3;
	}
	else if(psec_end_indices.size() != psec_start_indices.size())
	{
		cout << "Not the same psec start indices as end indices: " << psec_end_indices.size() << " and " << psec_start_indices.size() << endl;
		return 4;
	}
	else if(psec_end_indices.size() != metadata_end_indices.size())
	{
		cout << "Not the same psec end indices as metadata end indices: " << psec_end_indices.size() << " and " << metadata_end_indices.size() << endl;
		return 5;
	}
	else if((int)psec_end_indices.size() != NUM_PSEC)
	{
		cout << "Got " << (int)psec_end_indices.size() << " chip's worth of data when expecting " << NUM_PSEC << endl;
		return 6;
	}
	//-----------end error management--------//


	//raw data separated and indexed by psec chip number
	map<int, vector<unsigned short>> rawPsec;
	map<int, vector<unsigned short>> rawMeta;
	//special set of 10 or so words with some local
	//info from the ACC.
	vector<unsigned short> cc_header_info; 

	//blockify the buffer based on the found indices
	//by making a new vector that takes a chunk of the
	//raw buffer using vector iterators.
	for(int i = 0; i < (int)psec_end_indices.size(); i++)
	{

		vector<unsigned short>::const_iterator first = b.begin() + psec_start_indices.at(i) + 1;
		vector<unsigned short>::const_iterator last = b.begin() + psec_end_indices.at(i);
		vector<unsigned short> tempVec(first, last);
		rawPsec[i] = tempVec; //chip i's data is tempVec
	}

	//do the same for metadata
	for(int i = 0; i < (int)psec_end_indices.size(); i++)
	{

		vector<unsigned short>::const_iterator first = b.begin() + psec_end_indices.at(i) + 1;
		vector<unsigned short>::const_iterator last = b.begin() + metadata_end_indices.at(i);
		vector<unsigned short> tempVec(first, last);
		rawMeta[i] = tempVec; //chip i's data is tempVec
	}

	//similar for cc_header_info
	vector<unsigned short>::const_iterator first = b.begin();
	vector<unsigned short>::const_iterator last = b.begin() + psec_start_indices.at(0);
	vector<unsigned short> tempVec(first, last);
	cc_header_info = tempVec; 



	//restructure chip waveform data
	//to be indexed by channel. also apply
	//pedestal and linearity corrections.
	data.clear(); //the map indexed by channel
	int channelNo = 1; //count this as we loop
	for(int chip = 0; chip < NUM_PSEC; chip++)
	{
		vector<unsigned short> chipData = rawPsec[chip];
		int lastVal = (int)chipData.at(0); //just used for firmware development
		//make sure there is 256 samples per channel!
		if((int)chipData.size() != NUM_SAMP*NUM_CH_PER_CHIP)
		{
			cout << "Didn't get 256 samples per channel on chip " << chip << endl;
			cout << "Got a total of " << chipData.size() << " when expecting " << NUM_SAMP*NUM_CH_PER_CHIP << endl;
			//return 7;
		}
		for(int i = 0; i < NUM_CH_PER_CHIP; i++)
		{
			vector<unsigned short>::const_iterator first = chipData.begin() + i*NUM_SAMP;
			vector<unsigned short>::const_iterator last = chipData.begin() + (i+1)*NUM_SAMP;
			vector<unsigned short> tempVec(first, last);

			vector<double> waveform; //the double-cast, rescaled data
			for(int samp = 0; samp < (int)tempVec.size(); samp++)
			{
				//pedestal is a constant offset.
				//conversion (linearity) is a multiplicative scale
				waveform.push_back((tempVec.at(samp) - peds[channelNo][samp])*conv[channelNo][samp]);

				//checks on chronological order
				if(samp != 0 && lastVal + 1 != tempVec.at(samp))
				{
					cout << "Chronological issue at chip " << chip << " sample " << samp << endl;
					cout << "Last val: " << lastVal << ", this val " << tempVec.at(samp) << endl;
					return 7;
				}
				lastVal = tempVec.at(samp);
			}
			data[channelNo] = waveform;
			channelNo++;
		}
	}

	//parse the metadata buffer blocks in the Metadata class.
	meta.parseBuffer(rawMeta, cc_header_info);
	meta.setBoardAndEvent((unsigned short)boardIndex, eventNumber);
	
	return 0;

}




//writes data from the presently stored event
// to file assuming file has header already
void ACDC::writeDataToFile(ofstream& d, ofstream& m)
{
	string delim = " ";

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
	return;
}





//writes pedestals to file in a new
//file format relative to the old software. 
//<channel> <sample 1> <sample 2> ...
//<channel> ...
//samples in ADC counts. 
void ACDC::writePedsToFile(ofstream& ofs)
{
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
	
}


//reads pedestals to file in a new
//file format relative to the old software. 
//<channel> <sample 1> <sample 2> ...
//<channel> ...
//samples in ADC counts. 
void ACDC::readPedsFromFile(ifstream& ifs)
{
	char delim = ' '; //in between ADC counts
	map<int, vector<double>> tempPeds;//temporary holder for the new pedestal map

	//temporary variables for line parsing
	string lineFromFile; //full line
	string adcCountStr; //string representing adc counts of ped
	int ch; //int for the current channel key
	vector<double> tempWav; //ped wav temporary 
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
			tempWav.push_back(stod(adcCountStr)); //pedestal adcCounts
		}

		//now set this vector to the appropriate ped map element
		tempPeds.insert(pair<int, vector<double>>(ch, tempWav));
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








