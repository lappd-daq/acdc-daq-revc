#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <signal.h>
#include "ACC.h"

using namespace std;


//please do not use this script. it has not
//been tested nor completed.


//global variable for sigint capture
bool SIGFOUND = false;

void sigHandler(int s)
{
	cout << "got sigint " << s; //to remove compiler warning about unused variable
	SIGFOUND = true;
}

int dataQueryLoop(ofstream& dataofs, ofstream& metaofs, int nev, int trigMode)
{
	//setup a sigint capturer to safely
	//reset the boards if a ctrl-c signal is found
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sigHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	//initialize the ACC and ACDC objects, waking
	//up the USB line and making sure there are no issues. 
	cout << "---Starting data logging by checking for ACC and ACDC connectivity:";
	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects
	acc.softwareTrigger();
	bool success = acc.readAcdcBuffers(); //read ACDC buffer from usb, save and parse in ACDC objects

	acc.resetAccTrigger();
	acc.resetAccTrigger();

	//only print if you actually
	//got ACDC data. 
	if(!success)
	{
		cout << "Could not connect to ACDCs" << endl;
		return 0;
	}

	int evCounter = 0; //loop ends when this reaches nev
	int corruptCounter = 0;
	bool pullNewAccBuffer = true; //used when we need a new Acc buffer. 

	//duration variables
	auto start = chrono::steady_clock::now();
	auto end = chrono::steady_clock::now(); //just for initialization
	try
	{
		for(int evCounter = 0; evCounter < nev; evCounter++)
		{
			//close gracefull if ctrl-C is thrown
			if(SIGFOUND){throw("Ctrl-C detected, closing nicely");}

			cout << "On event " << evCounter << " of " << nev << "... ";

			//send a set of USB commands to configure ACC
			//send a software trigger
			acc.initializeForDataReadout(trigMode); 

			//tell the ACC to not send a trigger for a moment
			//(both trigger modes)
			acc.setAccTrigInvalid();

			int eventHappened = 2;
			//retval of readAcdcBuffers = 0 indicates
			//total success of pulling ACDC data. 
			while(eventHappened != 0)
			{
				//close gracefull if ctrl-C is thrown
				if(SIGFOUND){throw("Ctrl-C detected, closing nicely");}
				eventHappened = acc.readAcdcBuffers(); 
				if(eventHappened == 1)
				{
					corruptCounter++;
				}

				//this I think is required after reading data. 
				/*
				setAccTrigInvalid();
				setFreshReadmode();
				setAccTrigInvalid();
				setFreshReadmode();
				*/
			}


			end = chrono::steady_clock::now();
			cout << " found an event after waiting for a trigger. ";
			cout << "Computer time was " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " milliseconds. ";

			cout << "Writing the event to file" << endl;
			acc.writeAcdcDataToFile(dataofs, metaofs); //writes to file
			//head to top, incrementing event counter.
		}
	}
	catch(string mechanism)
	{
		cout << mechanism << endl;
		acc.dataCollectionCleanup();
		return 0;
	}

	end = chrono::steady_clock::now();
	cout << "Finished collecting " << nev << " events after " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " milliseconds. "<< endl;
	//clean up at the end
	acc.dataCollectionCleanup();
	cout << "Collected " << evCounter << " events and had to wade through " << corruptCounter << " number of corrupt buffers to get there" << endl;

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
	dataQueryLoop(dataofs, metaofs, nevents, trigMode);

	dataofs.close();
	metaofs.close();

	return 1;
}


