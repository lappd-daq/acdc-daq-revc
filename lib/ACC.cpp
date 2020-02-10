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
	usb = new stdUSB(vid, pid);
	if(!usb->isOpen())
	{
		cout << "Usb was unable to connect to ACC" << endl;
		delete usb;
		exit(EXIT_FAILURE);
	}
}

ACC::ACC(uint16_t vid, uint16_t pid)
{
	usb = new stdUSB(vid, pid);
	if(!usb->isOpen())
	{
		cout << "Usb was unable to connect to ACC" << endl;
		delete usb;
		exit(EXIT_FAILURE);
	}
}

ACC::~ACC()
{
	/*
	resetAccTrigger();
	readAcdcBuffers();
	resetAccTrigger();
	readAcdcBuffers();
	*/
	clearAcdcs();
	delete usb;
}


//this is "black magic". 
//for some reason (firmware or USB bug)
//the set-usb-readmode commands require 
//a number of sends and reads before 
//it returns any USB packets. 
//this is something to change when things get
//worked out. 
vector<unsigned short> ACC::sendAndRead(unsigned int command, int buffsize)
{
	if(!checkUSB()) exit(EXIT_FAILURE);

	int send_counter = 0;
	int max_sends = 50; //arbitrary. 
	bool loop_breaker = false;
	//this loop is because the ACC or USB
	//driver seems to need repetative
	//writing in order to properly get the
	//message across
	vector<unsigned short> tempbuff;
	while(!loop_breaker)
	{
		usb->sendData(command);
		send_counter++;
		tempbuff = usb->safeReadData(buffsize + 2);
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

//-----------The 0xB class of commands, the most cryptic

//(1) clearing the Acc instruction in firmware
//(2) sets Acc trigger = valid
//(3) CC_SYNC goes to 1 which makes the
//firmware wait 50,000 clocks before sending
//the usb message. 
void ACC::prepSync()
{
	unsigned int command = 0x000B0018;
	usb->sendData(command);
}

//(1) clearing the Acc instruction in firmware
//(2) sets Acc trigger = valid
//(3) CC_SYNC goes to 0 which means it
//will send the usb message immediately
void ACC::makeSync()
{
	unsigned int command = 0x000B0010;
	usb->sendData(command);
}

//(1) resets firmware in trigger and time
//(2) does software reset of transeivers.vhd
//(3) sets CC_INSTRUCTION to the command. 
//Previously called "manage_cc_fifo", but now
//calling it "resetAccTrigger"
void ACC::resetAccTrigger()
{
	unsigned int command = 0x000B0001;
	usb->sendData(command);
}


void ACC::setAccTrigValid()
{
	unsigned int command = 0x000B0006;
	usb->sendData(command);
}


//------------- end 0xB

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
	clearAcdcs();
	//create ACDC objects with their board numbers
	//loaded into alignedAcdcIndices in the
	//last function call. 
	for(int bi: alignedAcdcIndices)
	{
		cout << "Creating a new ACDC object for detected ACDC: " << bi << endl;
		ACDC* temp = new ACDC();
		temp->setBoardIndex(bi);
		acdcs.push_back(temp);
	}

}

//properly delete Acdc vector
//as each one was created by new. 
void ACC::clearAcdcs()
{
	for(int i = 0; i < (int)acdcs.size(); i++)
	{
		delete acdcs[i];
	}
	acdcs.clear();
}

vector<unsigned short> ACC::readAccBuffer()
{
	cout << "Attempting to read the ACC info buffer" << endl;
	if(!checkUSB()) exit(EXIT_FAILURE);

	//writing this tells the ACC to respond
	//with its own metadata
	unsigned int command = 0x1e0C0005; 
	vector<unsigned short> v_buffer = sendAndRead(command, CC_BUFFERSIZE);
	lastAccBuffer = v_buffer; //save as a private variable
	return v_buffer; //also return as an option
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

//for each connected board (ACDC in acdc's)
//print the last loaded metadata object.
void ACC::printAcdcInfo(bool verbose)
{
	for(ACDC* a: acdcs)
	{
		cout << "Metadata from ACDC #" << a->getBoardIndex() << endl;
		a->printMetadata(verbose);
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

	readAccBuffer(); //needs to be done before sending software trig?

	cout << "Sending a software trigger to boards ";
	for(int bi: boards) cout << bi << ", ";
	cout << endl;

	//turn this into an unsigned int mask. 
	unsigned short mask = vectorToUnsignedShort(boards);
	//the present version of firmware only allows
	//one to mask boards 0-3 (as opposed to 0-7). 
	//take this part out when moving to 8 board firmware. 
	mask = mask & 0x000F;

	cout << "Soft trig mask is "; 
	printByte(mask);
	cout << endl;

	//force the board to trigger on a certain 160MHz
	//clock cycle within the event. default is 0, and
	//cannot be more than 3. 
	bin = bin % 4;

	

	//send the command
	unsigned int command = 0x000E0000; 
	command = command | mask;

	//prep synchronization?
	//prepSync();
	usb->sendData(command);
	//make synchronization?
	//makeSync();

}

//sends a command to read the ACDC buffer
//and loads that buffer into memory as a private
//variable in the ACDC objects themselves. 
//then tells the ACDC objects to parse their
//own f*$&ing buffers. 
void ACC::readAcdcBuffers()
{
	cout << "Reading ACDC buffers" << endl;
	//assumes that the last ACC buffer is still relevant
	//and double-checks the connected board values. however,
	//only boards that have recently been software triggered
	//will have updated metadata/buffers. 

	//refreshed connected Acdc list (only parser, no usb packets sent)
	whichAcdcsConnected();

	//each ACDC needs its own USB packet sent. There is no
	//bulk transfer where it captures all data from all 
	//boards. 
	for(int bi: alignedAcdcIndices)
	{
		cout << "Reading board number " << bi << endl;
		unsigned int command = 0x1e0C0000; //base command for set readmode
		command = command | (unsigned int)(bi + 1); //which board to read
		//vector<unsigned short> acdc_buffer = sendAndRead(command, ACDC_BUFFERSIZE);
		usb->sendData(command);
		vector<unsigned short> acdc_buffer = usb->safeReadData(ACDC_BUFFERSIZE + 2);
		this_thread::sleep_for(chrono::milliseconds(1000));

		//save this as a private member of ACDC
		//by looping through our acdc vector
		//and checking each index (not optimal but)
		//who cares about 4 loop iterations. 
		for(ACDC* a: acdcs)
		{
			if(a->getBoardIndex() == bi)
			{
				cout << "Saving buffer (size " << acdc_buffer.size() << ") on board " << bi << endl;
				a->setLastBuffer(acdc_buffer); //also triggers parsing function
			}
		}
	}

	resetAccTrigger();
}