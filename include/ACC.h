#ifndef _ACC_H_INCLUDED
#define _ACC_H_INCLUDED

#include "Ethernet.h"
#include "CommandLibrary.h"

#include <cstdlib> 
#include <bitset> 
#include <sstream> 
#include <string> 
#include <thread> 
#include <algorithm> 
#include <thread> 
#include <fstream> 
#include <atomic> 
#include <signal.h> 
#include <unistd.h> 
#include <cstring>
#include <chrono> 
#include <iomanip>
#include <numeric>
#include <ctime>

using namespace std;

#define NUM_CH 30 //maximum number of channels for one ACDC board
#define MAX_NUM_BOARDS 8 // maxiumum number of ACDC boards connectable to one ACC 
#define ACCFRAME 32
#define PPSFRAME 16
#define PSECFRAME 7795

class ACC
{
public:
	//------------------------------------------------------------------------------------//
	//--------------------------------Constructor/Deconstructor---------------------------//
	// >>>> ID 1: Constructor
	ACC();
    // >>>> ID 2: Constructor with IP and port argument
    ACC(std::string ip, std::string port);
	// >>>> ID 3: Destructor
	~ACC();

	//------------------------------------------------------------------------------------//
	//--------------------------------Local return functions------------------------------//
	vector<unsigned short> ReturnRawData(){return raw_data;}
	vector<unsigned short> ReturnACCIF(){return acc_if;} 

	//------------------------------------------------------------------------------------//
	//-------------------------Local set functions for board setup------------------------//
	//-------------------Sets global variables, see below for description-----------------//
	void SetNumChCoin(unsigned int in){SELF_number_channel_coincidence = in;} //sets the number of channels needed for self trigger coincidence	
	void SetEnableCoin(int in){SELF_coincidence_onoff = in;} //sets the enable coincidence requirement flag for the self trigger
	void SetThreshold(unsigned int in){SELF_threshold = in;} //sets the threshold for the self trigger
	void SetPsecChipMask(vector<int> in){SELF_psec_chip_mask = in;} //sets the PSEC chip mask to set individual chips for the self trigger 
	void SetPsecChannelMask(vector<unsigned int> in){SELF_psec_channel_mask = in;} //sets the PSEC channel mask to set individual channels for the self trigger 
	void SetValidationStart(unsigned int in){validation_start=in;} //sets the validation window start delay for required trigger modes
	void SetValidationWindow(unsigned int in){validation_window=in;} //sets the validation window length for required trigger modes
	void SetTriggermode(int in){triggersource = in;} //sets the overall triggermode
	void SetSign(int in, int source) //sets the sign (normal or inverted) for chosen source
	{
		if(source==2)
		{
			ACC_sign = in;
		}else if(source==3)
		{
			ACDC_sign = in;
		}else if(source==4)
		{
			SELF_sign = in;
		}
	}
	void SetPPSRatio(unsigned int in){PPSRatio = in;} 
	void SetPPSBeamMultiplexer(int in){PPSBeamMultiplexer = in;} 

	//------------------------------------------------------------------------------------//
	//-------------------------Local set functions for board setup------------------------//
    //ID 4: Main init function that controls generalk setup as well as trigger settings//
	int InitializeForDataReadout(unsigned int boardmask = 0xFF, int triggersource = 0); 
	//ID 5: Set up the software trigger//
	int SetTriggerSource(unsigned int boardmask = 0xFF, int triggersource = 0); 
    //ID 6: Main listen fuction for data readout. Runs for 5s before retuning a negative//
	int listenForAcdcData(int trigMode, bool raw = false, string timestamp="invalidTime"); 
	//ID 7: Special function to check connected ACDCs for their firmware version// 
	void VersionCheck();
	//ID 8: Fires the software trigger//
	void GenerateSoftwareTrigger(); 
	//ID 9: Tells ACDCs to clear their ram.// 	
	void DumpData(unsigned int boardMask = 0xFF); 
	//ID 10
	void ResetACDC(); //resets the acdc boards
    //ID 11
	void ResetACC(); //resets the acdc boards 
    // >>>> ID 12: Switch PPS input to SMA
    void PPStoSMA();
    // >>>> ID 13: Switch PPS input to RJ45
    void PPStoRJ45();
    // >>>> ID 14: Switch Beamgate  input to SMA
    void BeamgatetoSMA();
    // >>>> ID 15: Switch Beamgate  input to RJ45
    void BeamgatetoRJ45();
    //ID 16
    void WriteErrorLog(string errorMsg);

private:
	//------------------------------------------------------------------------------------//
	//---------------------------------Load neccessary classes----------------------------//
    Ethernet* eth;
    CommandLibrary CML;

	//----------all neccessary global variables
	unsigned int command_address; //var: contain command address for ethernet communication
    unsigned long long command_value; //var: contain command value for ethernet communication
	int ACC_sign; //var: ACC sign (normal or inverted)
	int ACDC_sign; //var: ACDC sign (normal or inverted)
	int SELF_sign; //var: self trigger sign (normal or inverted)
	int SELF_coincidence_onoff; //var: flag to enable self trigger coincidence
	int triggersource; //var: decides the triggermode
	unsigned int SELF_number_channel_coincidence; //var: number of channels required in coincidence for the self trigger
	unsigned int SELF_threshold; //var: threshold for the selftrigger
	unsigned int validation_start; //var: validation start for some triggermodes
	unsigned int validation_window; //var: validation window for some triggermodes
	unsigned int PPSRatio; //var: defines the multiplication of the PPS value 
	int PPSBeamMultiplexer; //var: defines the multiplication of the PPS value 
	vector<int> alignedAcdcIndices; //number relative to ACC index (RJ45 port) corresponds to the connected ACDC boards
	vector<unsigned int> SELF_psec_channel_mask; //var: PSEC channels active for self trigger
	vector<int> SELF_psec_chip_mask; //var: PSEC chips actove for self trigger
	vector<unsigned short> raw_data; //var: raw data vector appended with (number of boards)*7795
	vector<unsigned short> acc_if; //var: a vector containing different information about the ACC and ACDC

    // >>>> ID 0: Sigint handling
	static void got_signal(int);
};

#endif
