#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <chrono>
#include <atomic>
#include "ACC.h"
#include <signal.h>
#include <unistd.h>
#include <cstring>

using namespace std;


std::atomic<bool> quit(false); //signal flag

void got_signal(int)
{
	quit.store(true);
}


//return:
//0 if failed total collection
//1 if succeeded total collection
int dataQueryLoop(ofstream& dataofs, ofstream& metaofs, int nev, int trigMode)
{
	//setup a sigint capturer to safely
	//reset the boards if a ctrl-c signal is found
	struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);

	//initialize the ACC and ACDC objects, waking
	//up the USB line and making sure there are no issues. 
	cout << "---Starting data logging by checking for ACC and ACDC connectivity:";
	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects


	acc.resetAccRamFlags();
	acc.resetAccRamFlags();

	

	int corruptCounter = 0; //classified as unsuccessful pulls of ACDC buffer
	int maxCorruptCounts = 1000; //if this many failed ACDC pulls occur, kill loop. 

	//duration variables
	auto start = chrono::steady_clock::now();
	auto end = chrono::steady_clock::now(); //just for initialization
	
	try
	{
		for(int evCounter = 0; evCounter < nev; evCounter++)
		{
			//close gracefull if ctrl-C is thrown
			if(quit.load())
			{
				cout << "Cought a Ctrl-C, cleaning up" << endl;
				acc.dataCollectionCleanup();
				return 0;
			}

			cout << "On event " << evCounter << " of " << nev << "... ";

			//send a set of USB commands to configure ACC
			//send a software trigger if trigMode = 0
			//currently doesn't support any other mode
			acc.initializeForDataReadout(trigMode);

			int eventHappened = 2;
			//retval of readAcdcBuffers = 0 indicates
			//total success of pulling ACDC data. 
			while(eventHappened != 0)
			{
				if(corruptCounter >= maxCorruptCounts)
				{
					cout << "Too many corrupt buffers" << endl;
					throw("Too many corrupt events");
				}

				//close gracefull if ctrl-C is thrown
				if(quit.load())
				{
					cout << "Cought a Ctrl-C, cleaning up" << endl;
					acc.dataCollectionCleanup();
					return 0;
				}
				eventHappened = acc.listenForAcdcData(trigMode, evCounter);
				if(eventHappened == 1)
				{
					corruptCounter++;
					acc.dataCollectionCleanup();
					acc.resetAccRamFlags();
					acc.resetAccRamFlags();
					acc.initializeForDataReadout(trigMode);
				}
				//this is a time-out because it seems
				//as if the ACDCs are no longer connected. 
				//Re-initialize
				if(eventHappened == 2)
				{
					corruptCounter++;
					cout << "Timed out, re-initializing" << endl;
					acc.dataCollectionCleanup();
					//acc.softReconstructor();
					acc.createAcdcs();
					acc.resetAccRamFlags();
					acc.resetAccRamFlags();
					acc.initializeForDataReadout(trigMode);
				}
				//sigint happened inside ACC class
				if(eventHappened == 3)
				{
					cout << "Cought a Ctrl-C, cleaning up" << endl;
					acc.dataCollectionCleanup();
					return 0;
				}

			}

			
			acc.setAccTrigInvalid();
			acc.setFreshReadmode();
			acc.setAccTrigInvalid();
			acc.setFreshReadmode();
			
			end = chrono::steady_clock::now();
			cout << "Found an event after waiting for a trigger. ";
			cout << "Computer time was " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " milliseconds. ";

			cout << "Writing the event to file" << endl;
			//writes to file. assumes no new AccBuffer 
			//has been pulled (i.e. uses fullRam from last AccBuffer)
			acc.writeAcdcDataToFile(dataofs, metaofs); 
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
	cout << "Found " << corruptCounter << " number of corrupt buffers during acquisition" << endl;

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

	cout << "Closing output files" << endl;
	dataofs.close();
	metaofs.close();

	return 1;
}


