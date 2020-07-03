#include "stdUSB.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>

using namespace std;

//this macro is a debugging function
//for sending and listening to data
//transmitted on the USB line. It circumvents
//the need for an ACC object and just isolates the usb.

//The function takes one argument, which is a
//string formatted as a hex code, like 0x1e0C0005
//which is sent down the line. Then it waits and
//reads the response, if any, providing information in stdout. 
//If you use multiple arguments separated by spaces,
//it will send each one and read in between (final argument = 1)
//or read only at the end (final argument = 0) 



//printing utility
void printReadBuffer(vector<unsigned short> b)
{
	 
	int counter = 0;
	//loop through each vector element
	cout << "Size of vector is " << b.size() << endl;
	for(unsigned short val: b)
	{
		cout << val << ", "; //decimal
		stringstream ss;
		ss << std::hex << val;
		string hexstr(ss.str());
		cout << hexstr << ", "; //hex
		unsigned n;
		ss >> n;
		bitset<16> word(n);
		cout << word.to_string(); //binary
		cout << "," << counter << "th word number" << endl;
		counter++;
	}
}


int main(int argc, char *argv[])
{
	
	stdUSB* usb = new stdUSB();
	if(!usb->isOpen())
	{
		cout << "Usb was unable to connect to ACC" << endl;
		delete usb;
		return 0;
	}

	usleep(100000);

	//number of 16 bit words to allocate memory for usb read
	int usbReadBufferMax = 10000;


	//if there is only one command
	if(argc == 2)
	{
		cout << "In this loop " << endl;
		unsigned int cmd;
		stringstream ss;
		ss << argv[1]; //parse char* into stringstream
		ss >> std::hex >> cmd; //interpret as a hex code
		usb->sendData(cmd); //send it down the usb line. 
		//wait a moment, very long compared to usb transfer
		usleep(300);
		vector<unsigned short> retval = usb->safeReadData(usbReadBufferMax);
		printReadBuffer(retval);
		cout << "Printed read buffer" << endl;
		return 0;
	}

	//loop over the number of args.
	for(int i = 1; i < (argc - 1); i++)
	{
		unsigned int cmd;
		stringstream ss;
		ss << argv[i]; //parse char* into stringstream
		ss >> std::hex >> cmd; //interpret as a hex code
		usb->sendData(cmd); //send it down the usb line. 
		usleep(300);
		//the final argument tells us whether to read 
		//in between commands or only read at the end. 
		if(atoi(argv[argc-1]) == 1)
		{
			vector<unsigned short> retval = usb->safeReadData(usbReadBufferMax);
			printReadBuffer(retval);
		}
	}

	//otherwise (0 or anything else on last argument)
	//read out at the end
	vector<unsigned short> retval = usb->safeReadData(usbReadBufferMax);
	printReadBuffer(retval);


	return 0;
}

