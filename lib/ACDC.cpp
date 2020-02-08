#include "ACDC.h"

using namespace std;

ACDC::ACDC()
{
	trigMask = 0xFFFFFF;
	convertMaskToChannels();
}

ACDC::ACDC(int bi)
{
	boardIndex = bi;
	trigMask = 0xFFFFFF;
	convertMaskToChannels();
}


ACDC::~ACDC()
{

}


void ACDC::setTriggerMask(unsigned int mask)
{
	trigMask = mask;
	convertMaskToChannels();
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