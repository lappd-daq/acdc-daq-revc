#ifndef _METADATA_H_INCLUDED
#define _METADATA_H_INCLUDED

#include <string>
#include <map>
#include <vector>

#define NUM_PSEC 5
#define NUM_CH 30

using namespace std;

class Metadata
{
public:
	Metadata();
	Metadata(vector<unsigned short> acdcBuffer); //if a buffer exists already to parse
	~Metadata();

	void parseBuffer(vector<unsigned short> acdcBuffer); 
	vector<int> getMaskedChannels(); //returns a vector format of masked channels
	void standardPrint(); //lite print
	void printAllMetadata(); //full raw print. 
	//two metadatas that are known externally need to be set by ACDC class.
	void setBoardAndEvent(unsigned short board, unsigned short event); 

private:
	map<string, unsigned short> metadata;

	vector<string> metadata_keys;
	void initializeMetadataKeys();
	void checkAndInsert(string key, unsigned short val); //inserts vals into metadata map. 
};


#endif