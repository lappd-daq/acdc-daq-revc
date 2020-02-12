#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "ACC.h"

using namespace std;


int dataQueryLoop(ofstream& dataofs, ofstream& metaofs, int nev, int trigMode)
{
	//initialize the ACC and ACDC objects, waking
	//up the USB line and making sure there are no issues. 
	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects
	acc.readAcdcBuffers(); //read ACDC buffer from usb, save and parse in ACDC objects

	return 1;
}




//checks if file exists by trying to
//open it as an ifstream. standard method
bool fileExists(const string fn)
{
	ifstream ifile(fn.c_str());
	return (bool)ifile;
}


//prints standard headers to the datafiles before
//logging data. 
void setupFileHeaders(ofstream& d, ofstream& m)
{
	string delim = " ";


	//----datafile header is as follows
	d << "Event" << delim << "Board" << delim << "Ch";
	//want to print an integer for each sample in PSEC chip. 
	//create a dummy ACDC object to get the number of expected samples
	ACDC tempacdc;
	int numSamp = tempacdc.getNumSamp();
	for(int i = 0; i < numSamp; i++)
	{
		d << delim << i;
	}
	d << endl;

	//----metafile header is as follows
	//we want to print the metadata_keys that represent
	//string keys to the metadata map. We want to do so 
	//in a consistent ordered way. Because maps are unordered,
	//there exists a metadata_keys vector in the Metadata class
	//that is constructed to keep the order of keys consistent. 

	//Create a dummy Metadata object. 
	Metadata tempmeta; //key vector is constructed in constructor
	tempmeta.printKeysToFile(m, delim); //prints metadata_keys to the ofstream.

	return;
}

//this function is the main data logging script
//Usage: ./bin/logData <path/to/outfilename> <nevents> <trigger mode 0-soft 1-hardware> 
int main(int argc, char *argv[]) {

	if(argc != 4)
	{
		cout << "Usage: ./bin/logData <path/to/outfilename> <nevents> <trigger mode 0-soft 1-hardware 2-rate only>" << endl;
		return 0;
	}

	int nevents = atoi(argv[2]);
	int trigMode = atoi(argv[3]);
	string outfilename = string(argv[1]);

	//filename logistics
	string datafn = outfilename + ".acdc";
	string metafn = outfilename + ".meta";

	//query the user to pick a name of file that
	//does not exist yet, or overwrite current file. 
	//only need to check datafile, as we assume they
	//come in pairs. 
	string temp;
	while(fileExists(datafn))
	{
		cout << "file already exists, try new filename (include file path): (or enter to overwrite / ctrl-C to quit): ";
        getline(cin, temp);
        if (temp.empty()) {break;}
        datafn = temp + ".acdc";
		metafn = temp + ".meta";
	}

	//open the files
	ofstream dataofs(datafn.c_str(), ios_base::trunc); //trunc overwrites
	ofstream metaofs(metafn.c_str(), ios_base::trunc); //trunc overwrites
	//write header info to both files
	setupFileHeaders(dataofs, metaofs);

	//start the loop that logs data and creates all
	//usb volitile objects.
	int retval = dataQueryLoop(dataofs, metaofs, nevents, trigMode);

	return 1;
}


