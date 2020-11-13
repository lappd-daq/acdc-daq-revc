#include "stdUSB.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>

using namespace std;

stdUSB* usb;

vector<unsigned short> sendAndRead(unsigned int command, int buffsize)
{

	int send_counter = 0; //number of usb sends
	int max_sends = 10; //arbitrary. 
	bool loop_breaker = false; 
	
	vector<unsigned short> tempbuff;
	while(!loop_breaker)
	{

		usb->sendData(command);
		send_counter++;
		tempbuff = usb->safeReadData(buffsize + 2);

		//this is a BAD failure that I have seen happen
		//if the USB line is flooded by the ACC. The 
		//result is that some c++ memory has been overwritten
		//(calloc buffer doesn't allocate enough bytes). 
		//You need to continue with a crazy large buffsize to
		//clear the USB line or multiple things will crash. 
		if((int)tempbuff.size() > buffsize + 2)
		{
			buffsize = 10*buffsize; //big enough to hold a whole ACDC buffer
			continue;
		}
		//if the buffer is non-zero size, then
		//we got a message back. break the loop
		if(tempbuff.size() > 0)
		{
			loop_breaker = true;
		}
		if(send_counter == max_sends)
		{
			loop_breaker = true;
		}
	}
	return tempbuff;
}

int main(int argc, char *argv[])
{
	
	usb = new stdUSB();
	vector<unsigned short> lastAccBuffer;

	unsigned int command = 0xFFB54000; // Disables Psec communication
	usb->sendData(command);

	usleep(10000);
	command = 0x000200FF; //Reserts RX buffer
	usb->sendData(command);
	command = 0xFFD00000; //Request a 32 word ACDC ID frame containing all important infomations
	usb->sendData(command);

	usleep(10000);

	command = 0x00200000; //Read the ID frame
	lastAccBuffer = sendAndRead(command, 20000);

	if(lastAccBuffer.size() != 32) //Check if buffer size is 32 words
	{
		cout << "Something wrong with ACC buffer, size: " << lastAccBuffer.size() << endl;
		return 0;
	}

	for(int i = 0; i < 8; i++)
	{
		if(lastAccBuffer.at(16+i) != 32){
			cout << "Board "<< i << " not with 32 words after ACC buffer read ";
			cout << "Size " << lastAccBuffer.at(16+i)  << endl;
			continue;
		}
		if(lastAccBuffer.at(16+i) == 32){
			cout << "Board "<< i << " with 32 words after ACC buffer read, ";
			cout << "Board "<< i << " connected" << endl;
			continue;
		}
	}
}
