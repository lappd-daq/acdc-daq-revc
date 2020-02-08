#include "ACC.h"
#include <cstdlib>
#include <bitset>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

ACC::ACC()
{
	uint16_t vid = 0x6672;
	uint16_t pid = 0x2920;
	stdUSB* tempusb = new stdUSB(vid, pid);
	if(!tempusb->isOpen())
	{
		cout << "Usb was unable to connect to ACC" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		usb = tempusb;
	}

	//the step that is "black magic"
	//to clear any historesis in the 
	//firmware/USB state from previous
	//sessions
	wakeup();

}

ACC::ACC(uint16_t vid, uint16_t pid)
{
	stdUSB* tempusb = new stdUSB(vid, pid);
	if(!tempusb->isOpen())
	{
		cout << "Usb was unable to connect to ACC" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		usb = tempusb;
	}

	//the step that is "black magic"
	//to clear any historesis in the 
	//firmware/USB state from previous
	//sessions
	wakeup();

}

//this is an empirically formed
//"wake-up" function that clears any
//historetical states in the firmware
//or USB line. I've found that this is needed
//before the first commands are sent to the board
//upon first starting the _software_. It has to
//do with the way the ACC is parsing USB bits
//and how the USB driver memory is deallocating. 
void ACC::wakeup()
{
	if(!usb->isOpen())
	{
		bool retval = usb->createHandles();
		if(!retval)
		{
			cout << "Cannot connect to ACC usb" << endl;
			exit(EXIT_FAILURE);
		}
	}
	//read firmware or programmers manual for explanation
	//this is me encapsulating poorly written firmware comms
	unsigned int command = 0x1e0C0005; 
	//writing this tells the ACC to respond
	//with its own metadata

	int send_counter = 0;
	int max_sends = 50; //arbitrary. 
	bool loop_breaker = false;
	int samples; //number of bytes actually read
	unsigned short* buffer;
	//this loop is because the ACC or USB
	//driver seems to need repetative
	//writing in order to properly get the
	//message across
	while(!loop_breaker)
	{
		usb->sendData(command);
		send_counter++;
	    buffer = (unsigned short*)calloc(CC_BUFFERSIZE + 2, sizeof(unsigned short));
		if(usb->readData(buffer, CC_BUFFERSIZE + 2, &samples))
		{
			loop_breaker = true;
		}
		if(send_counter == max_sends)
		{
			loop_breaker = true;
		}
	}
}

ACC::~ACC()
{
	delete usb;
}

bool ACC::checkUSB()
{
	if(!usb->isOpen())
	{
		bool retval = usb->createHandles();
		if(!retval)
		{
			cout << "Cannot connect to ACC usb" << endl;
			return false;
		}
	}
	else
	{
		return true;
	}
	return true;
}


//this queries the Acc buffer,
//finds which ACDCs are connected, then
//creates the ACDC objects. In this particular
//function, the ACDC objects are then
//filled with metadata by querying for 
//ACDC buffer readout. This action also
//allows for filling of ACC and ACDC private
// variables. The ACC object keeps
//the following info:
//Which ACDCs are LVDS aligned?
//Are there events in ACC ram and which ACDCs?
//"Digitizing start flag and DC_PKT" which are presently
//unknown to me -Evan (see packetUSB.vhd line 225)
void ACC::createAcdcs()
{
	readAccBuffer(); //loads a ACC buffer into private member
	//parses the last acc buffer to see which ACDCs are aligned. 
	whichAcdcsConnected(); 

	
	//clear the acdc vector
	acdcs.clear();
	//create ACDC objects with their board numbers
	//loaded into alignedAcdcIndices in the
	//last function call. 
	for(int bi: alignedAcdcIndices)
	{
		cout << "Creating a new ACDC object for detected ACDC: " << bi << endl;
		acdcs.push_back(ACDC(bi));
	}
}

vector<unsigned short> ACC::readAccBuffer()
{
	cout << "Attempting to read the ACC info buffer" << endl;
	if(!checkUSB()) exit(EXIT_FAILURE);

	//writing this tells the ACC to respond
	//with its own metadata
	unsigned int command = 0x1e0C0005; 
	
	int samples; //number of bytes actually read
	unsigned short* buffer; //this type required by usb driver
	buffer = (unsigned short*)calloc(CC_BUFFERSIZE + 2, sizeof(unsigned short)); //safe allocation
	usb->sendData(command);
	usb->readData(buffer, CC_BUFFERSIZE + 2, &samples); //read up to CC_BUFFFERSIZE + 2 bytes or less

	//fill buffer into a vector
	vector<unsigned short> v_buffer;
	//loop over each element in buffer
	for(int i = 0; i < samples; i++)
	{
		v_buffer.push_back(buffer[i]);
	}

	free(buffer); //free the calloc'ed memory
	lastAccBuffer = v_buffer;

	return v_buffer;
}

void ACC::printByte(unsigned short val)
{
	cout << val << ", "; //decimal
	stringstream ss;
	ss << std::hex << val;
	string hexstr(ss.str());
	cout << hexstr << ", "; //hex
	unsigned n;
	ss >> n;
	bitset<16> b(n);
	cout << b.to_string(); //binary
}

vector<int> ACC::whichAcdcsConnected(bool pullNew)
{
	if(!pullNew && lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	//for explanation of the "2", see INFO1 of CC_STATE in packetUSB.vhd
	unsigned short alignment_packet = lastAccBuffer.at(2); 
	//binary representation of this packet is 1 if the
	//board is connected for both the first two bytes
	//and last two bytes. 
	vector<int> connectedBoards;
	for(int i = 0; i < MAX_NUM_BOARDS; i++)
	{
		if((alignment_packet & (1 << i)))
		{
			//the i'th board is connected
			connectedBoards.push_back(i);
		}
	}

	//this allows no vector clearing to be needed
	alignedAcdcIndices = connectedBoards;
	return connectedBoards;
}

//returns map[board][is ram full]
map<int, bool> ACC::checkFullRamRegisters(bool pullNew)
{
	if(!pullNew && lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	unsigned short ram_packet = lastAccBuffer.at(4);
	map<int, bool> tempRamFull;

	//this will change when the full 8 bits of 
	//xram_full in packetUSB.vhd are included in the ACC buffer. 
	int ramRegisters = 4; 
	for(int i = 0; i < ramRegisters; i++)
	{
		if((ram_packet & (1 << i)))
		{
			//the i'th board is connected
			tempRamFull.insert(pair<int, bool>(i, true));
		}
	}

	//this allows no map-clearing to be needed
	fullRam = tempRamFull; //private variable storage
	return tempRamFull; //return as an alternative. 

}

//returns map[board][is digitizing flag true]
map<int, bool> ACC::checkDigitizingFlag(bool pullNew)
{
	if(!pullNew && lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	unsigned short dig_packet = lastAccBuffer.at(4);
	map<int, bool> tempDigFlag;

	//this will change when the full 8 bits of 
	//xram_full in packetUSB.vhd are included in the ACC buffer. 
	int bitsTransferred = 4; 
	for(int i = 0; i < bitsTransferred; i++)
	{
		if((dig_packet & (1 << i)))
		{
			//the i'th board is connected
			tempDigFlag.insert(pair<int, bool>(i, true));
		}
	}

	//this allows no map-clearing to be needed
	digFlag = tempDigFlag; //private variable storage
	return tempDigFlag; //return as an alternative. 
}

//returns map[board][is digitizing flag true]
map<int, bool> ACC::checkDcPktFlag(bool pullNew)
{
	if(!pullNew && lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	unsigned short dc_packet = lastAccBuffer.at(4);
	map<int, bool> tempDcPktFlag;

	//this will change when the full 8 bits of 
	//xram_full in packetUSB.vhd are included in the ACC buffer. 
	int bitsTransferred = 4; 
	for(int i = 0; i < bitsTransferred; i++)
	{
		if((dc_packet & (1 << i)))
		{
			//the i'th board is connected
			tempDcPktFlag.insert(pair<int, bool>(i, true));
		}
	}

	//this allows no map-clearing to be needed
	dcPkt = tempDcPktFlag; //private variable storage
	return tempDcPktFlag; //return as an alternative. 
}

void ACC::printRawAccBuffer(bool pullNew)
{
	//if pullNew is False but there
	//is no existing ACC buffer, one
	//must pull a new buffer. 
	if(!pullNew && lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	for(unsigned short val: lastAccBuffer)
	{
		printByte(val);
		cout << endl;
	}
	
}

//turns {0, 3, 6} into 0x00000049 (b1001001)
unsigned int ACC::vectorToUnsignedInt(vector<int> a)
{
	unsigned int result = 0x00000000;
	for(int val: a)
	{
		if(val < 32)
		{
			result = result | (1 << val);
		}
	}
	return result;
}

//turns {0, 3, 6} into 0x0049 (b1001001)
unsigned short ACC::vectorToUnsignedShort(vector<int> a)
{
	unsigned short result = 0x0000;
	for(int val: a)
	{
		if(val < 16)
		{
			result = result | (1 << val);
		}
	}
	return result;
}


//sends software trigger to all connected boards. 
//bin option allows one to force a particular 160MHz
//clock cycle to trigger on. anything greater than 3
//is defaulted to 0. 
void ACC::softwareTrigger(vector<int> boards, int bin)
{
	
	//default for boards is empty. If so, then
	//software trigger all active boards from last
	//buffer query. 
	if(boards.size() == 0)
	{
		boards = alignedAcdcIndices;
	}

	cout << "Sending a software trigger to boards ";
	for(int bi: boards) cout << bi << ", ";
	cout << endl;

	//turn this into an unsigned int mask. 
	unsigned int mask = vectorToUnsignedInt(boards);
	//the present version of firmware only allows
	//one to mask boards 0-3 (as opposed to 0-7). 
	//take this part out when moving to 8 board firmware. 
	mask = mask & 0x0000000F;

	//force the board to trigger on a certain 160MHz
	//clock cycle within the event. default is 0, and
	//cannot be more than 3. 
	bin = bin % 4;

	//send the command
	unsigned int command = 0x000E000; 
	command = command | mask | (1 << 4) | (bin << 5);

	usb->sendData(command);

}