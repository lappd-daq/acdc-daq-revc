#ifndef _METADATA_H_INCLUDED
#define _METADATA_H_INCLUDED

#include <string>
#include <map>
#include <vector>
#include <fstream>

#define NUM_PSEC 5
#define NUM_CH 30
#define NUM_CH_PER_PSEC 6

using namespace std;

class Metadata
{
public:
	Metadata();
	Metadata(vector<unsigned short> acdcBuffer); //if a buffer exists already to parse
	~Metadata();

	bool parseBuffer(vector<unsigned short> acdcBuffer); //returns success or fail 1/0

	//two metadatas that are known externally need to be set by ACDC class.
	void setBoardAndEvent(unsigned short board, int event); 
	int getEventNumber();
	void writeErrorLog(string errorMsg);
	map<string, unsigned short> getMetadata(){return metadata;}
	vector<string> getMetaKeys(){return metadata_keys;}
	void checkAndInsert(string key, unsigned short val); //inserts vals into metadata map. 

private:
	map<string, unsigned short> metadata;

	vector<string> metadata_keys;
	void initializeMetadataKeys();
	
};


#endif