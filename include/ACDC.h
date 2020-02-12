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
	void setLastBuffer(vector<unsigned short> b);
	void setPeds(map<int, vector<double>> p){peds = p;} //sets pedestal map
	void setConv(map<int, vector<double>> c){conv = c;} //sets adc-conversion map



private:
	int boardIndex;
	vector<unsigned short> lastAcdcBuffer;
	unsigned int trigMask;
	vector<int> maskedChannels;
	Metadata meta;
	map<int, vector<double>> data; //data[channel][waveform samples] channel starts at 1. 
	map<int, vector<double>> peds; //peds[channel][waveform samples] from a calibration file.
	map<int, vector<double>> conv; //conversion factor from adc counts to mv from calibration file. 

	void convertMaskToChannels();
	void fillData(); //parses the acdc buffer and fills the data map
	int getEventNumber(); 
};

#endif