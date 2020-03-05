#include <iostream>
#include "ACC.h"
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <atomic>

using namespace std;
//This function should be run
//every time new pedestals are
//set using the setConfig function. 
//this function measures the true
//value of the pedestal voltage applied
//by the DACs on-board. It does so by
//taking a lot of software trigger data
//and averaging each sample's measured
//voltage over all events. The data is then
//stored in an ascii text file and re-loaded
//during real data-taking. These pedestals are
//subtracted live during data-taking. 




std::atomic<bool> quit(false); //signal flag

void got_signal(int)
{
	quit.store(true);
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

//identical to the logData data query loop 
//except that it writes to a file in the
//calibration directory that represents the
//most recent pedestal calibration data. 
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


	acc.resetAccTrigger();
	acc.resetAccTrigger();

	

	int corruptCounter = 0; //classified as unsuccessful pulls of ACDC buffer
	int maxCorruptCounts = 10; //if this many failed ACDC pulls occur, kill loop. 
	bool raw = true; //to not apply the already text-loaded pedestals to the data acquisition
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

			//tell the ACC to not send a trigger for a moment
			//(both trigger modes)
			acc.setAccTrigInvalid();

			int eventHappened = 2;
			//retval of readAcdcBuffers = 0 indicates
			//total success of pulling ACDC data. 
			while(eventHappened != 0)
			{
				if(corruptCounter >= maxCorruptCounts)
				{
					throw("Too many corrupt events");
				}

				//close gracefull if ctrl-C is thrown
				if(quit.load())
				{
					cout << "Cought a Ctrl-C, cleaning up" << endl;
					acc.dataCollectionCleanup();
					return 0;
				}
				eventHappened = acc.listenForAcdcData(trigMode, evCounter, raw);
				if(eventHappened == 1)
				{
					corruptCounter++;
					acc.dataCollectionCleanup();
					acc.resetAccTrigger();
					acc.resetAccTrigger();
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
					acc.resetAccTrigger();
					acc.resetAccTrigger();
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

//Looks at the data that was just taken to calibrate pedestals
//and calculates average values for each board, saving into
//a file in the calibration directory, 
void calculatePedestalValues(ifstream& dataifs, string pedtag)
{

	//figure out which boards are active
	//in the dataset
	vector<int> activeBoards;
	string line; //full line in data file
	string word; //word in a line
	int bi; 
	char delim = ' ';
	int counter;
	int maxEvent; //highest event number in datafile
	getline(dataifs, line); //first line is a header, throw it out. 
	// a vector holding every line of the file. 
	//this is because getline throws the line
	//away and you cant go back. 
	vector<string> fileLines; 
	//loop through all lines and look
	//at board indices. 
	while(getline(dataifs, line))
	{
		fileLines.push_back(line); //save this line for later use.
		stringstream ssline(line); //stream parsing object
		counter = 0;
		while(getline(ssline, word, delim))
		{
			//event nmber is 1st column
			if(counter == 0)
			{
				maxEvent = stoi(word);
			}
			//board index is 2nd column
			if(counter == 1)
			{
				bi = stoi(word);
				break;
			}
			counter++;
		}
		if(activeBoards.size() == 0)
		{
			activeBoards.push_back(bi);
		}
		else
		{
			//is this element NOT in the vector? (== activeBoards.end())
			if(find(activeBoards.begin(), activeBoards.end(), bi) == activeBoards.end())
			{
				//add it to the vector
				activeBoards.push_back(bi);
			}
		}
	}


	//create acdc objects for each active board in the dataset
	vector<ACDC*> acdcs;
	for(int bi: activeBoards)
	{
		ACDC* temp = new ACDC();
		temp->setBoardIndex(bi);
		acdcs.push_back(temp);
	}


	int totEventsRead; //counts number of events actually read for avg ped calibration.
	map<int, vector<double>>::iterator mit; //general purpose map iterator
	vector<double>::iterator vit; //generall purpose vector iterator
	//load data from file for that event on
	//each active board. 
	for(ACDC* a: acdcs)
	{
		map<int, vector<double>> avgPed;
		cout << "On acdc " << a->getBoardIndex() << endl;
		totEventsRead = 0;
		//loop over all events. 
		for(int evno = 0; evno < maxEvent; evno++)
		{
			//this function searches the file
			//(either starting from beginning or
			//the present line to find an event-number
			//to board number match.)
			map<int, vector<double>> tempPed = a->readDataFromFile(fileLines, evno);
			//did data reading fail? would produce empty map
			if(tempPed.size() == 0)
			{
				//don't iterate totEventsRead, just move forward
				continue;
			}

			//otherwise, we got an event. add to the
			//avg pedestal map. 
			for(mit = tempPed.begin(); mit != tempPed.end(); ++mit)
			{
				//if this channel exists already
				if(avgPed.count(mit->first) > 0)
				{
					vector<double> chped = mit->second;
					for(int i = 0; i < (int)chped.size(); i++)
					{
						avgPed[mit->first][i] += chped[i];
					}
				}
				else
				{
					avgPed.insert(pair<int, vector<double>>(mit->first, mit->second));
				}
			}
			totEventsRead++;
		}


		//write this ped data to file.
		ofstream ofsAvgPed(pedtag+to_string(a->getBoardIndex())+".txt", ios::trunc);
		vector<double> tempwav; 
		int ch;
		for(mit = avgPed.begin(); mit != avgPed.end(); ++mit)
		{
			tempwav = mit->second;
			ch = mit->first;

			ofsAvgPed << ch << delim;//print channel to file with a delim
			for(vit = tempwav.begin(); vit != tempwav.end(); ++vit)
			{
				ofsAvgPed << (*vit/totEventsRead) << delim; //print ped value for that sample 
			}
			ofsAvgPed << endl;
		}
		ofsAvgPed.close();
	}

}



int main() {

	string datafn = CALIBRATION_DIRECTORY;
	datafn += "last_ped_calibration_data.acdc";
	string metafn = CALIBRATION_DIRECTORY;
	metafn += "last_ped_calibration_data.meta";
	string pedtag = CALIBRATION_DIRECTORY;
	string tag = PED_TAG;
	pedtag += tag; //file to ultimately save avg

	//open files that will hold the most-recent PED data. 
	ofstream dataofs(datafn.c_str(), ios_base::trunc); //trunc overwrites
	ofstream metaofs(metafn.c_str(), ios_base::trunc); //trunc overwrites
	//write header info to both files
	setupFileHeaders(dataofs, metaofs);

	//immediately enter a data collection loop
	//that will save data without pedestals subtracted
	//to get a measurement of the pedestal values. 
	int trigMode = 0; //software trigger for calibration
	int nevents = 100; //old software read 50 times for ped calibration. 
	dataQueryLoop(dataofs, metaofs, nevents, trigMode);

	dataofs.close();
	metaofs.close();


	//at this point, there are files with data that 
	//represents a measurement of the pedestals on each sample
	//and channel and board. We now analyze that data to get
	//averages and standard deviations. 

	//open infile stream for data file. 
	//(meta not needed here, but saved for
	//posterity sake)
	ifstream dataifs(datafn.c_str());
	if(!dataifs)
	{
		cout << "Something bad happened in file closing of calibration files" << endl;
		return 0;
	}

	calculatePedestalValues(dataifs, pedtag);



	return 1;
}