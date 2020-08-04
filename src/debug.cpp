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


	//number of 16 bit words to allocate memory for usb read
	int usbReadBufferMax = 50;
	int readflag; //argument specifying the read option.
	stringstream ss1;
	unsigned int cmd;
	ss1 << argv[argc-1];
	ss1 >> std::hex >> cmd;
	readflag = (int)cmd;
	//done formatting the readflag argument

	//if there is only one command
	if(argc == 2)
	{
		cout << "In this loop " << endl;
		stringstream ss;
		ss << argv[1]; //parse char* into stringstream
		ss >> std::hex >> cmd; //interpret as a hex code
		usb->sendData(cmd); //send it down the usb line. 
		//wait a moment, very long compared to usb transfer
		vector<unsigned short> retval = usb->safeReadData(usbReadBufferMax);
		printReadBuffer(retval);
		cout << "Printed read buffer" << endl;
		return 0;
	}
	else if(argc > 2 && !(readflag == 0 || readflag == 1))
	{
		cout << "For multiple messages, the last argument must be 0 or 1" << endl;
		return 0;
	}
	else
	{
		//loop over the number of args.
		for(int i = 1; i < (argc - 1); i++)
		{
			stringstream ss;
			ss << argv[i]; //parse char* into stringstream
			ss >> std::hex >> cmd; //interpret as a hex code
			usb->sendData(cmd); //send it down the usb line. 
			//the final argument tells us whether to read 
			//in between commands or only read at the end. 
			if(readflag == 1)
			{
				vector<unsigned short> retval = usb->safeReadData(usbReadBufferMax);
				printReadBuffer(retval);
			}
		}

		//otherwise (0 or anything else on last argument)
		//read out at the end
		if(readflag == 0)
		{
			vector<unsigned short> retval = usb->safeReadData(usbReadBufferMax);
			printReadBuffer(retval);
		}
	}
	


	return 0;
}

