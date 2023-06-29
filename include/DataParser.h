#ifndef _DATAPARSER_H_INCLUDED
#define _DATAPARSER_H_INCLUDED

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

using namespace std;

#define NUM_CH 30 //maximum number of channels for one ACDC board
#define NUM_PSEC 5 //maximum number of psec chips on an ACDC board
#define NUM_SAMP 256 //maximum number of samples of one waveform
#define NUM_CH_PER_CHIP 6 //maximum number of channels per psec chips

class DataParser
{
    public:
        DataParser(); //constructor
        ~DataParser(); //deconstructor

        //----------local return functions
        map<int, std::vector<unsigned short>> returnData(){return data;} //returns the entire data map | index: channel < samplevector
        vector<unsigned short> returnMetaData(){return meta;} //returns the entire meta map | index: metakey < value 

        //----------parse function for data stream 
        int parseDataFromBuffer(vector<unsigned short> buffer); //parses only the psec data component of the ACDC buffer
        int parseMetaFromBuffer(vector<unsigned short> buffer,int boardID);

        //----------write data to file
        void writeErrorLog(string errorMsg); //write errorlog with timestamps
        
        map<int, std::vector<unsigned short>> data;
        vector<unsigned short> meta;
    private:

};

#endif
