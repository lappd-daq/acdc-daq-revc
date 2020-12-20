#ifndef _ACDC_H_INCLUDEDreadDataFromFile
#define _ACDC_H_INCLUDED

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include "Metadata.h"

using namespace std;

#define NUM_CH 30
#define NUM_PSEC 5
#define NUM_SAMP 256
#define NUM_CH_PER_CHIP 6

class ACDC
{
public:
	ACDC();
	~ACDC();


	int getBoardIndex();
	void setBoardIndex(int bi);
	int getNumCh() {int a = NUM_CH; return a;}
	int getNumPsec() {int a = NUM_PSEC; return a;}
	int getNumSamp() {int a = NUM_SAMP; return a;}
	void setPeds(map<int, vector<double>> p, int bi){peds[bi] = p;} //sets pedestal map
	void setConv(map<int, vector<double>>& c){conv = c;} //sets adc-conversion map
	void setData(map<int, vector<double>>& d){data = d;} //sets data map
	int parseDataFromBuffer(vector<unsigned short> acdc_buffer); //parses only the psec data component of the ACDC buffer
	void writeRawBufferToFile(vector<unsigned short> lastAcdcBuffer);
	void writeRawDataToFile(vector<unsigned short> buffer, ofstream& d);
	void readPedsFromFile(ifstream& ifs, int bi);
	void readConvsFromFile(ifstream& ifs);
	void writeErrorLog(string errorMsg);

	map<int, vector<double>> returnData(){return data;}
	map<string, unsigned short> returnMeta(){return map_meta;}

private:
	int boardIndex;
	vector<unsigned short> lastAcdcBuffer;
	unsigned int trigMask;
	Metadata meta;
	map<int, vector<double>> data; //data[channel][waveform samples] channel starts at 1. 
	map<int, map<int, vector<double>>> peds; //peds[channel][waveform samples] from a calibration file.
	map<int, vector<double>> conv; //conversion factor from adc counts to mv from calibration file. 
	map<string, unsigned short> map_meta;
};

#endif
