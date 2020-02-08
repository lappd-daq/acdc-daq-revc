#ifndef _ACDC_H_INCLUDED
#define _ACDC_H_INCLUDED

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

using namespace std;

#define NUM_CH 30

class ACDC
{
public:
	ACDC();
	ACDC(int bi);
	~ACDC();

	void setTriggerMask(unsigned int mask);
	unsigned int getTriggerMask(); //get the trig mask (this syntax used)
	vector<int> getMaskedChannels(); //get this private var. 

private:
	int boardIndex;
	unsigned int trigMask;
	vector<int> maskedChannels;
	void convertMaskToChannels();
};

#endif