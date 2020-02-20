#ifndef _CONFIG_H_INCLUDED
#define _CONFIG_H_INCLUDED

#include "ACC.h"
#include <string>

#define NUM_BOARDS 8
#define NUM_CHIPS 5

using namespace std;

class Config
{
public:
	Config();
	Config(string filename, bool verbose);
	~Config();

	void setConfigFilename(string filename);

	void initializeDefaultValues(); //fills private vars with default values
	bool parseConfigFile(bool verbose); //does yaml parsing, filling private vars
	bool writeConfigToAcc(ACC* acc); //writes the config variables to an acc

	//utility for good printing of masked channels
	vector<int> getMaskedChannels(unsigned int mask);
	//the opposite, take a vector make unsigned int for ACDC setting. 
	unsigned int getChannelMaskFromVector(vector<int> vmask);


private:
	string configFilename;

	//------configuration variables
	unsigned int trig_mask[NUM_BOARDS];
	bool trig_enable[NUM_BOARDS];
	unsigned int pedestal[NUM_BOARDS][NUM_CHIPS];
	unsigned int threshold[NUM_BOARDS][NUM_CHIPS];
	bool trig_sign[NUM_BOARDS];
	bool wait_for_sys;
	bool rate_only;
	bool sma_trig_on_fe[NUM_BOARDS];
	bool hrdw_trig;
	bool hrdw_sl_trig;
	unsigned int hrdw_trig_src;
	unsigned int hrdw_sl_trig_src;
	unsigned int coinc_window;
	unsigned int coinc_pulsew;
	bool use_coinc;
	bool use_trig_valid;
	unsigned int coinc_num_ch;
	unsigned int coinc_num_asic;
};

#endif