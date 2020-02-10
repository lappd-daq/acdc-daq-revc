#include "ACDC.h"

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

void ACDC::setLastBuffer(vector<unsigned short> b)
{
	lastAcdcBuffer = b;
	meta.parseBuffer(b); //the BIG buffer parsing function that fills data/metadata maps

	int evNo = getEventNumber(); //some function that returns an event number. not sure how to get this yet. 
	meta.setBoardAndEvent((unsigned short)boardIndex, (unsigned short)evNo);
}

//looks at the last ACDC buffer and organizes
//all of the data into a data object. This is called
//explicitly so as to not allocate a bunch of memory 
//without need.  
void ACDC::fillData()
{
	//make sure an acdc buffer has been
	//filled. if not, there is nothing to be done.
	if(lastAcdcBuffer.size() == 0)
	{
		cout << "You tried to parse ACDC data without pulling an ACDC buffer" << endl;
		return;
	}

	//word that indicates the data is
	//about to start for each psec chip.
	unsigned short startword = 0xF005; 
	unsigned short endword = 0xBA11; //just for safety, uses the 256 sample rule. 


}

int ACDC::getEventNumber()
{
	return 0;
}













