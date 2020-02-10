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
	ACDC(int bi); //the constructor that is mainly used
	~ACDC();

	void setTriggerMask(unsigned int mask);
	unsigned int getTriggerMask(); //get the trig mask (this syntax used)
	int getBoardIndex();
	vector<int> getMaskedChannels(); //get this private var. 
	void setLastBuffer(vector<unsigned short> b);

private:
	int boardIndex;
	vector<unsigned short> lastAcdcBuffer;
	unsigned int trigMask;
	vector<int> maskedChannels;
	Metadata meta;
	map<int, vector<double>> data; //data[channel][waveform samples] channel starts at 1. 

	void convertMaskToChannels();
	void fillData(); //parses the acdc buffer and fills the data map
};

#endif