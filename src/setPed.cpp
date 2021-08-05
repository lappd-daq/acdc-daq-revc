#include <iostream>
#include "ACC.h"
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <map>
#include <numeric>

#define CALIBRATION_DIRECTORY "directory"
#define PED_TAG "pedtag"

using namespace std;

std::atomic<bool> quit(false); //signal flag

void got_signal(int)
{
	quit.store(true);
}


int main(int argc, char *argv[]){

	stdUSB* usb = new stdUSB();
	if(!usb->isOpen())
	{
		cout << "Usb was unable to connect to ACC" << endl;
		delete usb;
		return 0;
	}
	
	ACC* acc;
	if(argc == 2){
		cout << "-------------------------------------------------------" << endl;
		cout << "Only one input found using it for all boards and chips" << endl;
	}
	if(argc < 4 && argc != 2)
	{
		cout << "Usage: `./bin/setPed boardmask chipmask value`" <<endl;
		cout << "Boardmask from 0x00 to 0xFF with each bit representing one ACDC board" << endl;
		cout << "Chipmask from 00000 to 11111 with each bit representing one PSEC chip" << endl;
		cout << "Value from 0 to 4095 repreenting adc counts" << endl;
		return 0;
	}
	if(argc > 4)
	{
		cout << "Too many inputs, continuing regardless!" << endl;
	}

	unsigned int adc;
	unsigned int boardmask;
	unsigned int chipmask;

	if(argc ==2)
	{
		adc = strtoul(argv[1],NULL,10);
		boardmask = 0xFF;
		chipmask = 0x1F;
	}
	if(argc >= 4)
	{
		adc = strtoul(argv[3],NULL,10);
		boardmask = strtoul(argv[1],NULL,8);
		chipmask = strtoul(argv[2],NULL,2);
	}

	unsigned int command = 0xFFA20000;
	command = (command & (command | (boardmask << 24))) | (chipmask << 12) | adc;
	usb->sendData(command);
	
	printf("send 0x%08x\n",command);	

	//open files that will hold the most-recent PED data.
	for(int bi=0; bi<8; bi++)
	{
		string datafn = CALIBRATION_DIRECTORY;
		string mkdata = "mkdir -p ";
		mkdata += CALIBRATION_DIRECTORY;
		system(mkdata.c_str());
		datafn += PED_TAG; 
		datafn += "_board";

		datafn += to_string(bi);
		datafn += ".txt";
		ofstream dataofs(datafn.c_str(), ios_base::trunc); //trunc overwrites
		cout << "Generating " << datafn << " ..." << endl;

		string delim = " ";
		for(int enm=0; enm<256; enm++)
		{
			for(int ch=0; ch<30; ch++)
			{
				if(argc ==2)
				{
					dataofs << atoi(argv[1]) << " ";
				}else if(argc >= 4)
				{
					dataofs << atoi(argv[3]) << " ";
				}
			}
			dataofs << endl;
		}
		dataofs.close();
	}
	return 1;
}
