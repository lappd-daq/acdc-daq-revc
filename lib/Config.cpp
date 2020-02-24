#include "Config.h"

using namespace std;

#include "Config.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <math.h>
#include "stdUSB.h"
#include "yaml-cpp/yaml.h"

using namespace std;


Config::Config()
{
	initializeDefaultValues();
}

Config::Config(string filename, bool verbose)
{
	configFilename = filename;
	initializeDefaultValues();
	//because you gave a filename in constructor, 
	//just parse it off the bat. it is just parsing
	//yaml at this point. 
	parseConfigFile(verbose);
}

Config::~Config()
{

}


//a tool for turning unsigned int into a 
//vector of indices that have 1's or 0's. 
//this function is set to return "masked channels", 
//i.e. channels that are not active. 
vector<int> Config::getMaskedChannels(unsigned int mask)
{
	vector<int> masked_channels;

    for(int i = 1; i <= NUM_CH; i++)
    {
        //logic operate on the i'th bit.
        //if it is 0, it is a masked channel
        if((mask & (1 << i)) == 0)
        {
            masked_channels.push_back(i);
        }
    }

    return masked_channels;

}

//takes a vector of masked channels and returns the
//appropriate "active channels" unsigned int for setting
//the ACDC mask. The format for the ACDC packet that sets the
//mask is that "1" corresponds to an active channel. So it really
//converts to "active channels" as opposed to a mask. But the
//input should have only channel numbers that are to-be inactive. 
//Start channel numbering from 1
unsigned int Config::getChannelMaskFromVector(vector<int> vmask)
{
	unsigned int mask = 0xFFFFFFFF;
	for(int i: vmask)
	{
		mask = mask & (0 << i);
	}
	return mask;
}

//initialize all private variables (yamlconfurations)
//to reasonable values. Called in constructor. 
void Config::initializeDefaultValues() {
    for (int i = 0; i < NUM_BOARDS; i++) {
        for (int j = 0; j < NUM_CHIPS; j++) {
            pedestal[i][j] = 0x800;
            threshold[i][j] = 0x000;
        }

        trig_mask[i] = 0xFFFFFFFF;  // 32 bit
        trig_enable[i] = false;
        sma_trig_on_fe[i] = false;
        trig_sign[i] = 0;

    }

    wait_for_sys = false;
    rate_only = false;
    hrdw_trig = false;
    hrdw_sl_trig = false;
    hrdw_trig_src = 0;
    hrdw_sl_trig_src = 0;
    use_coinc = false;
    use_trig_valid = false;
    coinc_num_ch = 0;
    coinc_num_asic = 0;
    coinc_window = 0;
    coinc_pulsew = 0;
}


bool Config::parseConfigFile(bool verbose) {
	//the highest heirarchical node
	YAML::Node yamlconf = YAML::LoadFile(configFilename);

	//--------begin acdc settings block

	if (yamlconf["acdc_settings"]) 
	{
		//the block under acdc_settings 
		YAML::Node acdc_conf = yamlconf["acdc_settings"];
		int board, chip; //for indexing boards and chips
		if (verbose) {cout << "__________________________" << endl;}
	
		//set pedestal settings
		if (acdc_conf["pedestal"]) 
		{
			YAML::Node pedestals = acdc_conf["pedestal"];
			//loop over each pedestal
			for (YAML::const_iterator ped = pedestals.begin(); ped != pedestals.end(); ++ped) 
			{
				board = ped->first.as<int>();
				//each chip can have 6 pedestals.
				for (YAML::const_iterator chips = ped->second.begin(); chips != ped->second.end(); ++chips) 
				{
					//truncated text option all sets all chip
					//peds to be the same. 
					if (chips->first.as<string>() == "all") 
					{
						for (int c = 0; c < NUM_CHIPS; c++) 
						{
							pedestal[board][c] = chips->second.as<unsigned int>();
							if (verbose) {cout << "Pedestal on board " << board << " chip " << c << " set to " << pedestal[board][c] << " ADC counts " << endl;}
						}
						break; // short-circuit the chip loop
					} 
					else 
					{
						chip = chips->first.as<int>();
						pedestal[board][chip] = chips->second.as<unsigned int>();
						if (verbose) {cout << "Pedestal on board " << board << " chip " << chip << " set to " << pedestal[board][chip] << " ADC counts " << endl; }
					}
				}

			}

			if (acdc_conf["threshold"]) 
			{
				//thresholds node
				YAML::Node thresholds = acdc_conf["threshold"];
				//look to comments above in pedestal section. this is identical
				for (YAML::const_iterator thresh = thresholds.begin(); thresh != thresholds.end(); ++thresh) 
				{
					board = thresh->first.as<int>();
					for (YAML::const_iterator chips = thresh->second.begin(); chips != thresh->second.end(); ++chips) 
					{
						if (chips->first.as<string>() == "all") 
						{
							for (int c = 0; c < NUM_CHIPS; c++) 
							{
								threshold[board][c] = chips->second.as<unsigned int>();
								if (verbose) {cout << "Threshold on board " << board << " chip " << c << " set to " << threshold[board][c] << " ADC counts " << endl; }
							}
							break; // Short-circuit
						} 
						else 
						{
							chip = chips->first.as<int>();
							threshold[board][chip] = chips->second.as<unsigned int>();
							if (verbose) {cout << "Threshold on board " << board << " chip " << chip << " set to " << pedestal[board][chip] << " ADC counts " << endl;}
						}
					}
				}
			}
		}
		if (verbose) {cout << "__________________________" << endl;}
	}

	//--------end acdc settings block

	//--------begin trigger settings block

	if (yamlconf["trigger_settings"]) 
	{
		YAML::Node trig_conf = yamlconf["trigger_settings"];
		if (verbose) { cout << "__________________________" << endl;}
	    if (trig_conf["trig_mask"]) {
	        YAML::Node masks = trig_conf["trig_mask"];
	        for (YAML::const_iterator board = masks.begin(); board != masks.end(); ++board) {
	        	//this is where I would like to port in functionality for
	        	//having the yml file read "mask: 1 4 12 30" and then it will
	        	//call getChannelMaskFromVector to create an unsigned int for the ACDC
	            trig_mask[board->first.as<int>()] = board->second.as<unsigned int>();
	            if (verbose) {
	                cout << "trig_mask on board " << board->first.as<int>() << " set to 0x" << hex
	                     << board->second.as<unsigned int>() << endl;
	            }
	        }
	    }
	    if (trig_conf["trig_enable"]) {
	        YAML::Node flags = trig_conf["trig_enable"];
	        for (YAML::const_iterator board = flags.begin(); board != flags.end(); ++board) {
	            trig_enable[board->first.as<int>()] = board->second.as<bool>();
	            if (verbose) {
	                cout << "trig_enable on board " << board->first.as<int>() << " set to " << board->second.as<bool>()
	                     << endl;
	            }
	        }
	    }
	    if (trig_conf["trig_sign"]) {
	        YAML::Node signs = trig_conf["trig_sign"];
	        for (YAML::const_iterator board = signs.begin(); board != signs.end(); ++board) {
	            string sign = board->second.as<string>();
	            trig_sign[board->first.as<int>()] = sign == "rising";
	            if (verbose) {
	                cout << "trig_sign on board " << board->first.as<int>() << " set to " << sign << endl;
	            }
	        }
	    }
	    if (trig_conf["wait_for_sys"]) {
	        wait_for_sys = trig_conf["wait_for_sys"].as<bool>();
	        if (verbose) {
	            cout << "wait_for_sys set to " << wait_for_sys << endl;
	        }
	    }
	    if (trig_conf["rate_only"]) {
	        rate_only = trig_conf["rate_only"].as<bool>();
	        if (verbose) {
	            cout << "rate_only set to " << rate_only << endl;
	        }
	    }
	    if (trig_conf["hrdw_trig"]) {
	        hrdw_trig = trig_conf["hrdw_trig"].as<bool>();
	        if (verbose) {
	            cout << "hrdw_trig set to " << hrdw_trig << endl;
	        }
	    }
	    if (trig_conf["hrdw_trig_src"]) {
	        hrdw_trig_src = trig_conf["hrdw_trig_src"].as<unsigned int>();
	        if (verbose) {
	            cout << "hrdw_trig_src set to " << hrdw_trig_src << endl;
	        }
	    }
	    if (trig_conf["hrdw_sl_trig"]) {
	        hrdw_sl_trig = trig_conf["hrdw_sl_trig"].as<bool>();
	        if (verbose) {
	            cout << "hrdw_sl_trig set to " << hrdw_sl_trig << endl;
	        }
	    }
	    if (trig_conf["hrdw_sl_trig_src"]) {
	        hrdw_sl_trig_src = trig_conf["hrdw_sl_trig_src"].as<unsigned int>();
	        if (verbose) {
	            cout << "hrdw_sl_trig_src set to " << hrdw_sl_trig_src << endl;
	        }
	    }
	    if (trig_conf["sma_trig_on_fe"]) {
	        YAML::Node flags = trig_conf["sma_trig_on_fe"];
	        for (YAML::const_iterator board = flags.begin(); board != flags.end(); ++board) {
	            sma_trig_on_fe[board->first.as<int>()] = board->second.as<bool>();
	            if (verbose) {
	                cout << "sma_trig_on_fe on board " << board->first.as<int>() << " set to " << board->second.as<bool>()
	                     << endl;
	            }
	        }
	    }
	    if (trig_conf["use_coinc"]) {
	        use_coinc = trig_conf["use_coinc"].as<bool>();
	        if (verbose) {
	            cout << "use_coinc set to " << use_coinc << endl;
	        }
	    }
	    if (trig_conf["coinc_window"]) {
	        coinc_window = trig_conf["coinc_window"].as<unsigned int>();
	        if (verbose) {
	            cout << "coinc_window set to " << coinc_window << endl;
	        }
	    }
	    if (trig_conf["coinc_pulsew"]) {
	        coinc_pulsew = trig_conf["coinc_pulsew"].as<unsigned int>();
	        if (verbose) {
	            cout << "coinc_pulsew set to " << coinc_pulsew << endl;
	        }
	    }
	    if (trig_conf["coinc_num_ch"]) {
	        coinc_num_ch = trig_conf["coinc_num_ch"].as<unsigned int>();
	        if (verbose) {
	            cout << "coinc_num_ch set to " << coinc_num_ch << endl;
	        }
	    }
	    if (trig_conf["coinc_num_asic"]) {
	        coinc_num_asic = trig_conf["coinc_num_asic"].as<unsigned int>();
	        if (verbose) {
	            cout << "coinc_num_asic set to " << coinc_num_asic << endl;
	        }
	    }
	    if (trig_conf["use_trig_valid"]) {
	        use_trig_valid = trig_conf["use_trig_valid"].as<bool>();
	        if (verbose) {
	            cout << "use_trig_valid set to " << use_trig_valid << endl;
	        }
	    }
	}



	return true;
}

bool Config::writeConfigToAcc(ACC* acc) 
{
	cout << "Writing the configuration settings to the ACC" << endl;

	//first, make sure the ACDCs have a fresh trigger mode, 0
	acc->resetAcdcTrigger();
	vector<int> acdcIndices = acc->getAlignedIndices();
	unsigned int command;
	unsigned int tempWord;
	unsigned int boardAddress; //pretty high byte indexing the board 
	unsigned int chipAddress;
	bool failCheck = false;

	//now we can write directly to the usb line inside this class
	stdUSB* usb = acc->getUsbStream(); 

	//this is taken directly from the old software
	//which has all of this written in explicitly. 
	//Here is some dirtiness of the organization of
	//the acc/acdc firmware. See programmers manual
	try
	{
		for(int bi: acdcIndices)
		{
			cout << "Setting configurations for board " << bi << endl;
			boardAddress = ((1 << bi) << 25); //25 offset is where the ACC interprets these bytes. 

			cout << "Setting trigger settings" << endl;
			//set self trigger mask 
			command = 0x00060000; //lo component
			tempWord = 0x00007FFF & trig_mask[bi]; //picking out low component
			command = tempWord | command | boardAddress;
			failCheck = usb->sendData(command); //lo trig
			if(!failCheck){throw("Failed setting trig mask lo on board index " + to_string(bi));}
			command = 0x00068000; //hi component
			tempWord = (0x3FFF8000 & trig_mask[bi]) >> 15; //picking out hi component
			command = tempWord | command | boardAddress; 
			failCheck = usb->sendData(command); //hi trig
			if(!failCheck){throw("Failed setting trig mask hi on board index " + to_string(bi));}


			//self_trigger_lo
			command = 0x00070000;
			tempWord = (use_trig_valid << 6)
				| (use_coinc << 5)
				| (sma_trig_on_fe[bi] << 4) 
				| (trig_sign[bi] << 3)
				| (rate_only << 2) 
				| (wait_for_sys << 1)
				| trig_enable[bi] 
				| (coinc_window << 7)
				| boardAddress;
		    command = command | tempWord;
		    failCheck = usb->sendData(command); //set self trig lo
		    if(!failCheck){throw("Failed setting trig config lo on board index " + to_string(bi));}

		    //self_trigger_hi
		    command = 0x00078000;
		    tempWord = (coinc_num_ch << 6)
				| (coinc_num_asic << 3) 
				| coinc_pulsew
				| boardAddress;
			command = (command | (1 << 11)) | tempWord;
			failCheck = usb->sendData(command); //set self trig hi
			if(!failCheck){throw("Failed setting trig config hi on board index " + to_string(bi));}

			cout << "Setting pedestals and thresholds" << endl;
			//set pedestals and thresholds
			for(int chip = 0; chip < NUM_CHIPS; chip++)
			{
				chipAddress = (1 << chip) << 20; //20 magic number

				//pedestal
				command = 0x00030000;
				tempWord = pedestal[bi][chip] | boardAddress | chipAddress;
				command = command | tempWord;
				failCheck = usb->sendData(command); //set ped on that chip
				if(!failCheck){throw("Failed setting pedestal on board index " + to_string(bi) + " chip index " + to_string(chip));}

				//threshold
				command = 0x00080000;
				tempWord = threshold[bi][chip] | boardAddress | chipAddress;
				command = command | tempWord;
				failCheck = usb->sendData(command); //set thresh on that chip
				if(!failCheck){throw("Failed setting threshold on board index " + to_string(bi) + " chip index " + to_string(chip));}
			}
		}

		cout << "Setting trigger source" << endl;
		//set trigger mode
		acc->setHardwareTrigSrc(hrdw_trig_src);
	}
	catch(string message)
	{
		cout << message << endl;
		return false;
	}

	
	return true;
}