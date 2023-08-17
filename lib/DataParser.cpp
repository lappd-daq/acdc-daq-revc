#include "DataParser.h"

DataParser::DataParser(){}

DataParser::~DataParser(){}

int DataParser::parseDataFromBuffer(vector<unsigned short> buffer)
{
    //Catch empty buffers
	if(buffer.size() == 0){return 0;}

	if(buffer.size() == 16)
	{
		data.clear();
		return -1;	
	}

	//Prepare the Metadata vector 
	data.clear();
    int channel_count = 0;

	//Indicator words for the start/end of the metadata
	const unsigned short startword = 0xF005; 
	unsigned short endword = 0xBA11;
	unsigned short endoffile = 0x4321;

	//Empty vector with positions of aboves startword
	vector<int> start_indices = {2, 1554, 3106, 4658, 6210}; 

	//Find the startwords and write them to the vector
	vector<unsigned short>::iterator bit;

	//Fill data map
	for(int i: start_indices)
	{
		//Write the first word after the startword
		bit = buffer.begin() + (i+1);
		//As long as the endword isn't reached copy metadata words into a vector and add to map
		vector<unsigned short> InfoWord;
		while(*bit != endword && *bit != endoffile)
		{
			InfoWord.push_back((unsigned short)*bit);
			if(InfoWord.size()==NUM_SAMP)
			{
				data.insert(pair<int, vector<unsigned short>>(channel_count, InfoWord));
				InfoWord.clear();
				channel_count++;
			}
			++bit;
		}	
	}

	return 0;
}

int DataParser::parseMetaFromBuffer(vector<unsigned short> buffer, int boardID)
{
    //Catch empty buffers
    if(buffer.size() == 0){return 0;}

    //Prepare the Metadata vector and helpers
    std::vector<unsigned int> meta;
    int chip_count = 0;

    //Indicator words for the start/end of the metadata
    unsigned short endword = 0xFACE;
    unsigned short endoffile = 0x4321;

    //Empty metadata map for each Psec chip <PSEC #, vector with information>
    map<int, vector<unsigned short>> PsecInfo;
    map<int, vector<unsigned short>> PsecTriggerInfo;
    unsigned short CombinedTriggerRateCount;

    //Empty vector with positions of aboves startword
    vector<int> start_indices= {1539, 3091, 4643, 6195, 7747};

    //Find the startwords and write them to the vector
    vector<unsigned short>::iterator bit;

    //Fill the psec info map
    for(int i: start_indices)
    {
        //Write the first word after the startword
        bit = buffer.begin() + (i+1);
        //As long as the endword isn’t reached copy metadata words and add to map
        vector<unsigned short> InfoWord;
        while(*bit != endword && *bit != endoffile && InfoWord.size() < 14)
        {
            InfoWord.push_back(*bit);
            ++bit;
        }
        PsecInfo.insert(pair<int, vector<unsigned short>>(chip_count, InfoWord));
        chip_count++;
    }

    //Fill the psec trigger info map
    for(int chip=0; chip<NUM_PSEC; chip++)
    {
        for(int ch=0; ch<NUM_CH/NUM_PSEC; ch++)
        {
            //Find the trigger data at begin + last_metadata_start + 13_info_words + 1_end_word + 1
            bit = buffer.begin() + start_indices[4] + 13 + 1 + 1 + ch +
            (chip*(NUM_CH/NUM_PSEC));
            PsecTriggerInfo[chip].push_back(*bit);
        }
    }

    //Fill the combined trigger
    CombinedTriggerRateCount = buffer[7792];

    //----------------------------------------------------------
    //Start the metadata parsing
    meta.push_back(boardID);
    for(int CHIP=0; CHIP<NUM_PSEC; CHIP++)
    {
    meta.push_back((0xDCB0 | CHIP));
    for(int INFOWORD=0; INFOWORD<13; INFOWORD++)
    {
    meta.push_back(PsecInfo[CHIP][INFOWORD]);
    }
    for(int TRIGGERWORD=0; TRIGGERWORD<6; TRIGGERWORD++)
    {
    meta.push_back(PsecTriggerInfo[CHIP][TRIGGERWORD]);
    }
    }
    meta.push_back(CombinedTriggerRateCount);
    meta.push_back(0xeeee);

    return 1;
}



void DataParser::writeErrorLog(string errorMsg)
{
    string err = "errorlog.txt";
    cout << "------------------------------------------------------------" << endl;
    cout << errorMsg << endl;
    cout << "------------------------------------------------------------" << endl;
    ofstream os_err(err, ios_base::app);
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    //ss << std::put_time(std::localtime(&in_time_t), "%m-%d-%Y %X");
    os_err << "------------------------------------------------------------" << endl;
    os_err << ss.str() << endl;
    os_err << errorMsg << endl;
    os_err << "------------------------------------------------------------" << endl;
    os_err.close();
}









