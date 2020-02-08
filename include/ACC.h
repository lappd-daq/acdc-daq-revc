#ifndef _ACC_H_INCLUDED
#define _ACC_H_INCLUDED

#include "ACDC.h"
#include "stdUSB.h"

using namespace std;

#define CC_BUFFERSIZE 32
#define MAX_NUM_BOARDS 8

class ACC
{
public:
	ACC();
	ACC(uint16_t vid, uint16_t pid); 
	~ACC();

	//parsing functions (no usb comms)
	void printAccMetadata(bool pullNew = false);
	void printRawAccBuffer(bool pullNew = false);
	map<int, bool>  checkFullRamRegisters(bool pullNew = false);
	map<int, bool>  checkDigitizingFlag(bool pullNew = false);
	map<int, bool>  checkDcPktFlag(bool pullNew = false);
	unsigned int vectorToUnsignedInt(vector<int> a); 
	unsigned short vectorToUnsignedShort(vector<int> a);

	
	//functions that involve usb comms
	void createAcdcs(); //creates ACDC objects, explicitly querying both buffers

	//sends software trigger to all connected boards. 
	//bin option allows one to force a particular 160MHz
	//clock cycle to trigger on. anything greater than 3
	//is defaulted to 0. 
	void softwareTrigger(vector<int> boards = {}, int bin = 0); 


private:
	stdUSB* usb;
	vector<unsigned short> lastAccBuffer; //most recently received ACC buffer
	vector<int> alignedAcdcIndices; //number relative to ACC index (RJ45 port)
	vector<ACDC> acdcs; //a vector of active acdc boards. 
	map<int, bool> fullRam; //which boards (first index) have full ram
	map<int, bool> digFlag; //which boards (first index) have a dig start flag (dont know what that means)
	map<int, bool> dcPkt; ////which boards (first index) have a dc packet (dont know what that means)


	//functions that involve usb comms
	vector<unsigned short> readAccBuffer();


	//parsing functions (no usb comms)
	vector<int> whichAcdcsConnected(bool pullNew = false);
	void printByte(unsigned short val);
	void wakeup(); //wakes the usb line, only called in constructor. 
	bool checkUSB(); //checking usb line and returning or couting appropriately.  

};

#endif