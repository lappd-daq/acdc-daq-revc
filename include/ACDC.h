#ifndef _ACDC_H_INCLUDED
#define _ACDC_H_INCLUDED

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include "Metadata.h"

using namespace std;

#define NUM_CH 30
#define NUM_PSEC 5
#define NUM_SAMP 256
#define NUM_CH_PER_CHIP 6

class ACDC
{
public:
	ACDC();
	~ACDC();


	void printMetadata(bool verbose = false);

	void setTriggerMask(unsigned int mask);
	unsigned int getTriggerMask(); //get the trig mask (this syntax used)
	int getBoardIndex();
	void setBoardIndex(int bi);
	int getNumCh() {int a = NUM_CH; return a;}
	int getNumPsec() {int a = NUM_PSEC; return a;}
	int getNumSamp() {int a = NUM_SAMP; return a;}
	vector<int> getMaskedChannels(); //get this private var. 
	void setPeds(map<int, vector<unsigned short>>& p){peds = p;} //sets pedestal map
	void setConv(map<int, vector<double>>& c){conv = c;} //sets adc-conversion map
	void setData(map<int, vector<unsigned short>>& d){data = d;} //sets data map
	int parseDataFromBufferInactive(vector<unsigned short> b, int eventNumber = 0); //parses raw data into metadata and psec data objects
	int parseDataFromBuffer(vector<unsigned short> b, int eventNumber = 0);
	void writeDataToFile(ofstream& d, ofstream& m); //writes data and metadata to filestream
	void writeRawBufferToFile();
	void printByte(ofstream& ofs, unsigned short val);
	void writePedsToFile(ofstream& ofs);
	void writeConvsToFile(ofstream& ofs);

	map<int, vector<unsigned short>> readDataFromFile(vector<string>, int evno); //takes a datafile and loads the data member with evno's data. 





private:
	int boardIndex;
	vector<unsigned short> lastAcdcBuffer;
	unsigned int trigMask;
	vector<int> maskedChannels;
	Metadata meta;
	map<int, vector<unsigned short>> data; //data[channel][waveform samples] channel starts at 1. 
	map<int, vector<unsigned short>> peds; //peds[channel][waveform samples] from a calibration file.
	map<int, vector<double>> conv; //conversion factor from adc counts to mv from calibration file. 

	void convertMaskToChannels();
	void fillData(); //parses the acdc buffer and fills the data map
};

#endif