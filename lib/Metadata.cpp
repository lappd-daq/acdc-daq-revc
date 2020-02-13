#include "Metadata.h"
#include <algorithm>
#include <sstream>
#include <bitset>
#include <iostream>
#include <cmath>

using namespace std;

Metadata::Metadata()
{
	initializeMetadataKeys();
}

Metadata::Metadata(vector<unsigned short> acdcBuffer)
{
	initializeMetadataKeys();
	parseBuffer(acdcBuffer);
}

Metadata::~Metadata()
{
}

//prints the raw metadata bytes
//in decimal, hex, and binary
void Metadata::printAllMetadata()
{

    vector<string>::iterator kit;
    for(string k: metadata_keys)
    {

        cout << k << "\t" << metadata[k] << "\t"; //decimal rep
        stringstream ss;
        ss << std::hex << metadata[k]; 
        string hexstr(ss.str());
        cout << hexstr << "\t"; //hex rep
        unsigned n;
        ss >> n;
        bitset<16> b(n);
        cout << b.to_string() << endl; //binary rep 
    }
    cout << endl;
    return;
}

void Metadata::writeMetadataToFile(ofstream& m, string delim)
{
    for(string k: metadata_keys)
    {
        m << metadata[k] << delim;
    }
    m << endl;
}
//this function prints a one line string of the metadata_keys
//vector to an ofstream "m" with delimiters delim. used
//in the data logging / metadata logging to ascii functions. 
void Metadata::printKeysToFile(ofstream& m, string delim)
{
    for(string k: metadata_keys)
    {
        m << k << delim;
    }
    m << endl;
    return;
}

//prints some relevant metadat to the
//standard cout. 
void Metadata::standardPrint()
{
	vector<int> masked_channels = getMaskedChannels();





	cout << "CC_EVENT_COUNT_LO:" << metadata["CC_EVENT_COUNT_LO"] << ", ";
	cout << "CC_TIME (nclocks, hi:mid:lo)  " << metadata["CC_TIMESTAMP_HI"] << ":" << metadata["CC_TIMESTAMP_MID"] << ":" << metadata["CC_TIMESTAMP_LO"] << endl;
	cout << "--------" << endl;
	cout << "acdc trigger time (nclocks, hi:mid:lo) " << metadata["trig_time_hi"] << ":" << metadata["trig_time_mid"] << ":" << metadata["trig_time_lo"] << endl;
	cout << "acdc total event count: " << (metadata["acdc_total_event_count_hi"]*pow(2, 16) + metadata["acdc_total_event_count_lo"]) << endl;
	cout << "acdc digitized event count: " << (metadata["digitized_event_count_hi"]*pow(2, 16) + metadata["digitized_event_count_lo"]) << endl;
	cout << "self-trig masked channels: ";

	vector<int>::iterator it;
	for(it = masked_channels.begin(); it != masked_channels.end(); ++it)
	{
		cout << to_string(*it) << ",";
	}

	cout << endl;
	cout << "self-trig enable: " << metadata["self_trig"] << endl;
	cout << "trig sign:        " << metadata["trigger_sign"] << endl;
	cout << "wait for sys:     " << metadata["wait_for_sys"] << endl;
	cout << "rate mode:        " << metadata["rate_mode"] << endl;
	cout << "on board sma trig " << metadata["onboard_acdc_sma_trig"] << endl;
	cout << "use chan coincidence   " << metadata["use_coincidence"] << endl;
	cout << "num channel coinc      " << metadata["channel_coincidence_min"] << endl;
	cout << "num asic coinc         " << metadata["asic_coincidence_min"] << endl;
	cout << "coincidence window     " << metadata["coincidence_window"] << endl;



	double ref_volt_mv = 1200;
	double num_bits = 4096;

	for (int i = 0; i < NUM_PSEC; i++) {
		cout << "PSEC:" << i;
		cout << "|ADC clock/trgt:" << metadata["ro_cnt"+to_string(i)] * 10 * pow(2, 11) / (pow(10, 6));
		cout << "/" << metadata["ro_target_cnt"+to_string(i)] * 10 * pow(2, 11) / (pow(10, 6)) << "MHz";
		cout << ",ro-bias:" << metadata["ro_dac_value"+to_string(i)] * ref_volt_mv / num_bits << "mV";
		cout << "|Ped:" << dec << metadata["vbias"+to_string(i)] * ref_volt_mv / num_bits << "mV";
		cout << "|Trig:" << metadata["trigger_threshold"+to_string(i)] * ref_volt_mv / num_bits << "mV";
		cout << endl;
	}

}


//parses the metadata map to return a vector
//form of the masked channels. 
vector<int> Metadata::getMaskedChannels()
{
	vector<int> masked_channels;

	//check to make sure the elements exist. if they
	//dont, then metadata hasnt been parsed yet.
	if(metadata.count("trig_mask_hi") == 0 || metadata.count("trig_mas_lo") == 0)
	{
		return masked_channels; //return empty vector
	}

    unsigned long mask = (metadata["trig_mask_hi"] << 16) + metadata["trig_mask_lo"];
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











//----------Massive parsing functions are below. 



//two metadatas that are known externally need to be set by ACDC class.
void Metadata::setBoardAndEvent(unsigned short board, int event)
{
    //there is a problem in that the map has been
    //defined assuming all metadata are unsigned shorts. 
    //but this only goes to 2^16 events (65536). If this happens, 
    //restart the count at zero
    event = event % 65536;
	checkAndInsert("Event", (unsigned short)event);
	checkAndInsert("Board", board);
}

int Metadata::getEventNumber()
{
    return (int)metadata["Event"];
}




//takes the full acdcBuffer as input. 
//splits it into a map[psec4][vector<unsigned shorts>]
//which is then parsed and organized in this class. 
bool Metadata::parseBuffer(vector<unsigned short> acdcBuffer)
{
	//if the buffer is 0 length (i.e. bad usb comms)
	//return doing nothing
	if(acdcBuffer.size() == 0) return false;
	

	//byte that indicates the metadata of
	//each psec chip is about to follow. 
	const unsigned short startword = 0xBA11; 

	//to hold data temporarily. local to this function. 
	//this is used to match to my old convention
	//from the old software (which took a while just)
	//to type out properly. 
	//ac_info[chip][index in buffer]
	map<int, vector<unsigned short>> ac_info;

	//this is the header to the ACDC buffer that
	//the ACC appends at the beginning, found in
	//packetUSB.vhd. 
	vector<unsigned short> cc_header_info;

	
	//indices of elements in the acdcBuffer
	//that correspond to the byte ba11
	vector<int> start_indices; 
	vector<unsigned short>::iterator bit;

	//loop through the data and find locations of startwords. 
    //this can be made more efficient if you are having efficiency problems.
	for(bit = acdcBuffer.begin(); bit != acdcBuffer.end(); ++bit)
	{
		//the iterator is at an element with startword value. 
		//push the index (integer, from std::distance) to a vector. 
        if(*bit == startword)
        {
             start_indices.push_back(std::distance(acdcBuffer.begin(), bit));
        }
	}

	//re-use this iterator to fill the cc_header_info vector
	//which starts with the first occurance of 1234. This will
	//be before the first occurance of the ac_info startword. 
	unsigned int cc_header_start = 0x1234;
	bit = std::find(acdcBuffer.begin(), acdcBuffer.end(), cc_header_start);
	bit++; //so that first byte is not the cc_header_start word. 
    int counter = 0;
    int maxAccInfo = 20; //the most bytes that would possibly be needed. 
	for(bit = bit; bit != acdcBuffer.end(); ++bit)
	{
		cc_header_info.push_back(*bit);
		
        //just fill with more than enough bytes. 
        //we will only access the relevant ones. 
		if(counter == maxAccInfo)
		{
			break;
		}
        counter++;
	}

	//I have found experimentally that sometimes
    //the ACC sends an ACDC buffer that has 8001 elements
    //(correct) but has BAD data, i.e. with extra ADC samples
    //and missing PSEC metadata (startwords). This event
    //needs to be thrown away really, but here all I can 
    //do is return and say that the metadata is nothing. 
	if(start_indices.size() != NUM_PSEC)
	{
        cout << "***********************************************************" << endl;
		cout << "In parsing ACDC buffer, found " << start_indices.size() << " matadata flag bytes." << endl;
		cout << "Metadata for this event will likely be jarbled. Code a protection!" << endl;
        return false;
	}

    //aparently this can happen, have not had enough 
    //trials to debug. 
    if(start_indices.size() == 0)
    {
        for(unsigned short v: acdcBuffer)
        {
            cout << std::hex << v << endl;
        }
        return false;
    }

	//loop through each startword index and store metadata. 
	int chip_count = 0;
	unsigned short endword = 0xFACE; //end of info buffer. 
	for(int i: start_indices)
	{
		//re-use buffer iterator from above
		//to set starting point. 
		bit = acdcBuffer.begin() + i + 1; //the 1 is to start one element after the startword
		//while we are not at endword, 
		//append elements to ac_info
		vector<unsigned short> infobytes;
		while(*bit != endword)
		{
			infobytes.push_back(*bit);
			++bit;
		}
		ac_info.insert(pair<int, vector<unsigned short>>(chip_count, infobytes));
		chip_count++;
	}

	//-----start the rutheless decoding of the 
	//-----acdc firmware and acc firmware packets. 
    for(int i = 0; i < NUM_PSEC; i++)
    {
        //indexing directly from ACDC top-level diagram and "lvds_com.vhd" line 316
        //ac_info[chip][0]: wilkcount
        //ac_info[chip][1]: readRO_cnt
        //ac_info[chip][2]: biases
        //ac_info[chip][3]: thresholds
        //ac_info[chip][4]: ro_vdd
        //ac_info[chip][10]: vcdl_count lo
        //ac_info[chip][11]: vcdl_count hi
        //ac_info[chip][12]: dll vdd
        checkAndInsert("ro_cnt"+to_string(i), ac_info[i][0]);
        checkAndInsert("ro_target_cnt"+to_string(i), ac_info[i][1]);
        checkAndInsert("vbias"+to_string(i), ac_info[i][2]);
        checkAndInsert("trigger_threshold"+to_string(i), ac_info[i][3]);
        checkAndInsert("ro_dac_value"+to_string(i), ac_info[i][4]);
        checkAndInsert("vcdl_count_lo"+to_string(i), ac_info[i][10]);//16 bit
        checkAndInsert("vcdl_count_hi"+to_string(i), ac_info[i][11]);//16 bit
        checkAndInsert("dll_vdd"+to_string(i), ac_info[i][12]);//16 bit

    }
    
    //indexing directly from ACDC top-level diagram and "lvds_com.vhd" line 316
    //ac_info[chip][5]: info_t - TRIGGER_INFO
    //ac_info[chip][6]: info_s - SAMPLE_INFO
    //ac_info[chip][7]: evt_cnt - XEVENT_COUNT
    //ac_info[chip][8]: last_instruct
    //ac_info[chip][9]: timestamp_and_instruct 


    //decoding info_t
    unsigned short info_t_0 = ac_info[0][5];
    unsigned short info_t_1 = ac_info[1][5];
    unsigned short info_t_2 = ac_info[2][5];
    unsigned short info_t_3 = ac_info[3][5]; //and info_t_4 is unused in firmware (see psec4_trigger_global)
    //parsing info_t_0
    unsigned short bin_count_save = info_t_0 & 0x0F;
    unsigned short self_trigger_settings = info_t_0 & 0xFFF0; //11 bit word 
    unsigned short cc_trigger_width = self_trigger_settings & 0x7; //first 3 bits
    unsigned short asic_coincidence_min = self_trigger_settings & 0x38; //next 3 bits
    unsigned short channel_coincidence_min = self_trigger_settings & 0x7C0; //next 5 bits
    //parsing info_t_1
    unsigned short num_triggered_channels = info_t_1 & 0x7C00; //the number of channels triggered 0-31
    unsigned short trigger_settings_0 = info_t_1 & 0x7FF; 
    unsigned short self_trig = trigger_settings_0 & 1; //is self trigger enabled, 1 or 0
    unsigned short sys_trig = trigger_settings_0 & 2; //is sys trig enabled, 1 or 0
    unsigned short rate_only = trigger_settings_0 & 4; //is rate counting mode enabled, 1 or 0
    unsigned short trigger_sign = trigger_settings_0 & 8; //1 for rising edge, 0 for falling
    unsigned short onboard_acdc_sma_trig = trigger_settings_0 & 16; //use the onboard sma as a trigger signal
    unsigned short use_coincidence = trigger_settings_0 & 32; //use coincidence between channels as a condition
    unsigned short use_trig_valid_as_reset = trigger_settings_0 & 64; //reset ACDC on trig valid
    unsigned short coincidence_window = trigger_settings_0 & 0x3C0; //coincidence window in number of clocks

    checkAndInsert("bin_count", bin_count_save);
    checkAndInsert("num_triggered_channels", num_triggered_channels);
    checkAndInsert("self_trig", self_trig);
    checkAndInsert("wait_for_sys", sys_trig);
    checkAndInsert("rate_mode", rate_only);
    checkAndInsert("trigger_sign", trigger_sign);
    checkAndInsert("reset_on_trig_valid", use_trig_valid_as_reset);
    checkAndInsert("onboard_acdc_sma_trig", onboard_acdc_sma_trig);
    checkAndInsert("use_coincidence", use_coincidence);
    checkAndInsert("coincidence_window", coincidence_window);
    checkAndInsert("channel_coincidence_min", channel_coincidence_min);
    checkAndInsert("asic_coincidence_min", asic_coincidence_min);
    checkAndInsert("cc_trigger_width", cc_trigger_width);


    //parsing info_t_2
    //self trig reset time is wierd. I think it may count
    //how many clock cycles it takes to fully reset the trigger. 
    //would have thought only one. see line 617 of psec4_trigger_Global
    unsigned short self_trig_reset_time_lo = info_t_2; //16 bit words
    unsigned short self_trig_reset_time_hi = info_t_3; 
    checkAndInsert("self_trig_reset_duration_lo", self_trig_reset_time_lo);
    checkAndInsert("self_trig_reset_duration_hi", self_trig_reset_time_hi);

    //pasing of info_s
    unsigned short info_s_0 = ac_info[0][6];
    unsigned short info_s_1 = ac_info[1][6];
    unsigned short info_s_2 = ac_info[2][6];
    unsigned short info_s_3 = ac_info[3][6]; 
    //a bitwise list of channels that had their discriminators
    //above threshold during the event, regardless of coincidence
    //logic or trigger mask; at the lowest level
    unsigned short triggered_channels_lo = info_s_0; //first 16 channels
    unsigned short triggered_channels_hi = info_s_1 & 0x1FFF; //second set of 14 channels
    checkAndInsert("triggered_channels_lo", triggered_channels_lo);
    checkAndInsert("triggered_channels_hi", triggered_channels_hi);
    unsigned short sma_bin_count_save_lo = info_s_1 & 0x6000; //not sure, but has to do with sma trigger. 
    //I think this is when ACC sends a trigger signal but the 
    //ACDC did not self trigger (i.e. in wait_for_sys mode)
    //good for characterizing efficiency
    unsigned short sys_trig_count_no_local_lo = info_s_2; 
    unsigned short sys_trig_count_no_local_hi = info_s_3;
    checkAndInsert("sys_but_no_local_hi", sys_trig_count_no_local_hi);
    checkAndInsert("sys_but_no_local_lo", sys_trig_count_no_local_lo);

    //parsing evt_count
    unsigned short evt_count_0 = ac_info[0][7]; 
    unsigned short evt_count_1 = ac_info[1][7]; 
    unsigned short evt_count_2 = ac_info[2][7]; 
    unsigned short evt_count_3 = ac_info[3][7]; 
    unsigned short evt_count_4 = ac_info[4][7]; 

    unsigned short firmware_resets_lo = evt_count_0;
    unsigned short firmware_resets_hi = evt_count_1;
    unsigned short firmware_version = evt_count_2;
    checkAndInsert("firmware_version", firmware_version);
    checkAndInsert("firmware_resets_hi", firmware_resets_hi);
    checkAndInsert("firmware_resets_lo", firmware_resets_lo);
    unsigned short self_trigger_mask_lo = evt_count_3; //channel mask first 16 channels
    unsigned short self_trigger_mask_hi = evt_count_4 & 0x1FFF; //second set of 14 channels
    checkAndInsert("trig_mask_hi", self_trigger_mask_hi);
    checkAndInsert("trig_mask_lo", self_trigger_mask_lo);
    unsigned short sma_bin_count_save_hi = evt_count_4 & 0x6000; //second two bits of sma bin count save
    //combine the sma_bin_count_saves
    unsigned short sma_bin_count_save = (sma_bin_count_save_hi << 2) + sma_bin_count_save_lo;
    checkAndInsert("sma_bin_count", sma_bin_count_save);

    //last instruction parsing
    unsigned short last_instruct_0 = ac_info[0][8];
    unsigned short last_instruct_1 = ac_info[1][8];
    unsigned short last_instruct_2 = ac_info[2][8];
    unsigned short last_instruct_3 = ac_info[3][8];
    unsigned short last_instruct_4 = ac_info[4][8];
    //this timestamp "latched_dig_time" is the time at
    //which the last digitized trigger was digitized
    unsigned short latched_dig_time_lo = last_instruct_0; //16 bit
    unsigned short latched_dig_time_mid = last_instruct_1; //16 bit
    unsigned short latched_dig_time_hi = last_instruct_2 & 0xFF; //8 bits
    checkAndInsert("readout_time_lo", latched_dig_time_lo);
    checkAndInsert("readout_time_mid", latched_dig_time_mid);
    checkAndInsert("readout_time_hi", latched_dig_time_hi);
    //I think this is the time from when a trigger signal is registered
    //to when the event (all channels) have been digitized. i.e. ~4 mu-s
    unsigned short valid_to_dig_time = last_instruct_2 & 0xFF00; // 8 bits. 
    checkAndInsert("readout_duration", valid_to_dig_time);
    unsigned short digitized_evt_count_lo = last_instruct_3; //16 bits counting how many events are digitized
    unsigned short digitized_evt_count_hi = last_instruct_4; //16 bits counting how many events are digitized
    checkAndInsert("digitized_event_count_lo", digitized_evt_count_lo);
    checkAndInsert("digitized_event_count_hi", digitized_evt_count_hi);


    //timestamp_and_instruct parsing
    unsigned short t_and_i_0 = ac_info[0][9];
    unsigned short t_and_i_1 = ac_info[1][9];
    unsigned short t_and_i_2 = ac_info[2][9];
    unsigned short t_and_i_3 = ac_info[3][9];
    unsigned short t_and_i_4 = ac_info[4][9];

    //latched system clock is time at which the trigger
    //signal on the FPGA went high
    unsigned short trig_time_lo = t_and_i_0; //16 bit
    unsigned short trig_time_mid = t_and_i_1; //16 bit
    unsigned short trig_time_hi = t_and_i_2; //16 bit
    checkAndInsert("trig_time_lo", trig_time_lo);
    checkAndInsert("trig_time_mid", trig_time_mid);
    checkAndInsert("trig_time_hi", trig_time_hi);
    //16 bit clock counter of all "events" which
    //are not necessarily digitization events. For example,
    //receiving some words from the ACC may be considered
    //an event. The details are a little fuzzy. 
    unsigned short acdc_total_event_count_lo = t_and_i_3; //16 bit 
    unsigned short acdc_total_event_count_hi = t_and_i_4; //16 bit 
    checkAndInsert("acdc_total_event_count_lo", acdc_total_event_count_lo);
    checkAndInsert("acdc_total_event_count_hi", acdc_total_event_count_hi);

    //CC clock count is three 16 bit words. Literally counting 40MHz clock cycles (not 125MHz, 25MHz, but 40Mhz from pll)
    checkAndInsert("CC_TIMESTAMP_LO", cc_header_info[3]);
    checkAndInsert("CC_TIMESTAMP_MID", cc_header_info[4]);
    checkAndInsert("CC_TIMESTAMP_HI", cc_header_info[5]);
    //event count is two 16 bit words, this combines them into an int
    checkAndInsert("CC_EVENT_COUNT_LO", cc_header_info[2]);
    checkAndInsert("CC_EVENT_COUNT_HI", cc_header_info[1]);
    //called in firmware "BIN_COUNT_SAVE" in triggerAndTime.vhd line 112. 
    //also contained in this buffer element is bin_count_start and bin_count. 
    checkAndInsert("CC_BIN_COUNT", (cc_header_info[0] & 0x18) >> 3);

	return true;
}


//just makes sure not to insert elements
//into metadata map if they already exist. 
void Metadata::checkAndInsert(string key, unsigned short val)
{
	//if the key exists, change the value
    if(metadata.count(key) > 0)
    {
        metadata[key] = val;
    }
    //if it is new, insert
    else
    {
        metadata.insert(pair<string, unsigned short>(key, val));
    }
    return;
}



//keeps the metadata strings in a consistent
//order. Initialized in constructor and is used
//to order the printing/output of metadata map. 
void Metadata::initializeMetadataKeys()
{
	metadata_keys.push_back("Event"); metadata_keys.push_back("Board");
	metadata_keys.push_back("CC_TIMESTAMP_HI");
	metadata_keys.push_back("CC_TIMESTAMP_MID"); metadata_keys.push_back("CC_TIMESTAMP_LO");
	metadata_keys.push_back("CC_EVENT_COUNT_HI");metadata_keys.push_back("CC_EVENT_COUNT_LO"); 
    metadata_keys.push_back("CC_BIN_COUNT");
	for(int i = 0; i < NUM_PSEC; i++)
	{
		metadata_keys.push_back("ro_cnt"+to_string(i));
		metadata_keys.push_back("ro_target_cnt"+to_string(i));
		metadata_keys.push_back("vbias"+to_string(i));
		metadata_keys.push_back("trigger_threshold"+to_string(i));
		metadata_keys.push_back("ro_dac_value"+to_string(i));
		metadata_keys.push_back("vcdl_count_lo"+to_string(i));
		metadata_keys.push_back("vcdl_count_hi"+to_string(i));
		metadata_keys.push_back("dll_vdd"+to_string(i));
	}

	//times and durations
	metadata_keys.push_back("trig_time_hi"); metadata_keys.push_back("trig_time_mid");
	metadata_keys.push_back("trig_time_lo"); metadata_keys.push_back("readout_duration");
	metadata_keys.push_back("readout_time_hi"); metadata_keys.push_back("readout_time_mid");
	metadata_keys.push_back("readout_time_lo"); metadata_keys.push_back("self_trig_reset_duration_hi");
	metadata_keys.push_back("self_trig_reset_duration_lo");

	//event counters and typifiers
	metadata_keys.push_back("acdc_total_event_count_hi");metadata_keys.push_back("acdc_total_event_count_lo");
	metadata_keys.push_back("digitized_event_count_hi");metadata_keys.push_back("digitized_event_count_lo");
	metadata_keys.push_back("sys_but_no_local_hi");metadata_keys.push_back("sys_but_no_local_lo");

	//event properties
	metadata_keys.push_back("num_triggered_channels"); metadata_keys.push_back("triggered_channels_hi");
	metadata_keys.push_back("triggered_channels_lo");metadata_keys.push_back("triggered_channels_lo");
	metadata_keys.push_back("firmware_resets_hi");metadata_keys.push_back("firmware_resets_lo");
	//settings (from configuration file)
	metadata_keys.push_back("self_trig"); metadata_keys.push_back("wait_for_sys");
	metadata_keys.push_back("rate_mode"); metadata_keys.push_back("trigger_sign");
	metadata_keys.push_back("reset_on_trig_valid");metadata_keys.push_back("trig_mask_hi");
	metadata_keys.push_back("trig_mask_lo");metadata_keys.push_back("firmware_version");
	metadata_keys.push_back("onboard_acdc_sma_trig"); metadata_keys.push_back("use_coincidence");
	metadata_keys.push_back("coincidence_window"); metadata_keys.push_back("channel_coincidence_min");
	metadata_keys.push_back("asic_coincidence_min"); metadata_keys.push_back("cc_trigger_width");

	//bin counts
	metadata_keys.push_back("sma_bin_count"); metadata_keys.push_back("bin_count");

}