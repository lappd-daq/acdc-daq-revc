#ifndef _ACC_H_INCLUDED
#define _ACC_H_INCLUDED

#include "ACDC.h"
#include "stdUSB.h"

using namespace std;

//These CC and ACDC buffer-sizes are inputs to a hard-coded
//wait statement in the stdUSB class during read and write operations.
//I am often re-tuning them to change the readout rate. A known
//issue exists where buffers returned by the ACC and ACDC are not
//always the size as expected from properly operating firmware or usb system. 
//Hence why these buffer-sizes are inflated so that calloc'ed memory is not
//written past its allocation. 
#define NUM_CH 30
#define CC_BUFFERSIZE 100 //will only be 64 if all goes as expected from firmware
#define ACDC_BUFFERSIZE 18000 //will only be 16002 if all goes as expected from firmware
#define SAFE_BUFFERSIZE 100000
#define MAX_NUM_BOARDS 8
#define CALIBRATION_DIRECTORY "./autogenerated_calibrations/"
#define PED_TAG "PEDS_ACDC"
#define LIN_TAG "LUT_ACDC-index-"

class ACC
{
public:
	ACC();
	~ACC(); 

	//----------parsing functions (no usb comms)
	unsigned int vectorToUnsignedInt(vector<int> a); //utility for going from list to 101011 bits. 
	unsigned short vectorToUnsignedShort(vector<int> a);
	vector<int> unsignedShortToVector(unsigned short a);
	void writeEDataToFile(vector<unsigned short> acdc_buffer); 
	vector<int> getAlignedIndices(){return alignedAcdcIndices;} //returns vector of aligned acdc indices
	string getCalDirectory(){return CALIBRATION_DIRECTORY;}
	string getPedTag(){return PED_TAG;}
	string getLinTag(){return LIN_TAG;}
	void printByte(unsigned short val, int format);
	//-----------functions that involve usb comms
	//(see cpp declaration for more comments above functions)
	int createAcdcs(); //creates ACDC objects, explicitly querying both buffers
	void setSoftwareTrigger(vector<int> boards = {}); //prepares software trigger and sets on acc/acdc
	void softwareTrigger(); //sends the software trigger
	void enableTransfer(int onoff=0);
	void toggleCal(int onoff, unsigned int channelmask = 0x7FFF); //toggles calibration input switch on boards
	int readAcdcBuffers(bool raw = false, string timestamp ="invalidTime", int oscopeOnOff=0); //reads the acdc buffers
	int listenForAcdcData(int trigMode, bool raw = false, string timestamp="invalidTime",int oscopeOnOff=0); //almost identical to readAcdcBuffers but intended for real data triggering
	int initializeForDataReadout(int trigMode = 0,unsigned int boardMask = 0xFF, int calibMode = 0);
	void dumpData(); //tells ACDCs to clear their ram
	bool setPedestals(unsigned int ped, vector<int> boards = {});
  	void emptyUsbLine(); //attempting to remove the crashes due to non-empty USB lines at startup.
	void writeErrorLog(string errorMsg);
	void writePsecData(ofstream& d); 

	//-----short usb send functions. found
	//-----at the end of the cpp file. 
	void setHardwareTrigSrc(int src, unsigned int boardMask = 0xFF); 
	//--reset functions
	void usbWakeup(); //40EFF;
	void resetACDCs(); //4F000;
	stdUSB* getUsbStream(); //returns the private usb object

	//Set functions for trigger
	void setDetectionMode(int in){detectionMode = in;}
	void setInvertMode(int in){invertMode = in;}
	void setChCoin(unsigned int in){ChCoin = in;}	
	void setEnableCoin(int in){enableCoin = in;}
	void setThreshold(int in){threshold = in;}
	void setTriggermode(int in){trigMode = in;}
	int getTriggermode(){return trigMode;}
	
	map<int, map<int, vector<double>>> returnPedData(){return map_data;}

private:
	stdUSB* usb;
	Metadata meta;
	vector<unsigned short> lastAccBuffer; //most recently received ACC buffer
	vector<int> alignedAcdcIndices; //number relative to ACC index (RJ45 port)
	vector<ACDC*> acdcs; //a vector of active acdc boards. 
	vector<int> boardsDoneTransferring; //which boards have finished sending all of their data to the ACC. 
	vector<int> boardsTransferring; //which boards have started sending data to the ACC but have not yet filled the RAM. 
	int detectionMode;
	int invertMode;
	int enableCoin;
	int trigMode;
	unsigned int ChCoin;
	unsigned int threshold;
	map<int, map<int, vector<double>>> map_data;
	map<int, map<string, unsigned short>> map_meta;

	//-----------functions that involve usb comms
	vector<unsigned short> readAccBuffer();


	//-----------parsing functions (no usb comms)
	vector<int> whichAcdcsConnected();
	void printByte(unsigned short val, char* format);
	vector<unsigned short> sendAndRead(unsigned int command, int buffsize); //wakes the usb line, only called in constructor. 
	bool checkUSB(); //checking usb line and returning or couting appropriately.  
	void clearAcdcs(); //memory deallocation for acdc vector. 
	int parsePedsAndConversions(); //puts ped and LUT-scan data into ACDC objects

	static void got_signal(int);
};

#endif
