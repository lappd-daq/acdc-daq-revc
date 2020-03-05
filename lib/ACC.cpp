#include "ACC.h"
#include <cstdlib>
#include <bitset>
#include <sstream>
#include <string>
#include <thread>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <cstring>

using namespace std;

//sigint handling
std::atomic<bool> quitacc(false); //signal flag

void ACC::got_signal(int)
{
	quitacc.store(true);
}
//



ACC::ACC()
{
	usb = new stdUSB();
	if(!usb->isOpen())
	{
		cout << "Usb was unable to connect to ACC" << endl;
		delete usb;
		exit(EXIT_FAILURE);
	}

	emptyUsbLine();
}


ACC::~ACC()
{
	cout << "Calling acc destructor" << endl;
	clearAcdcs();
	delete usb;
}


//sometimes it makes sense to repeatedly send
//a usb command until a response is sent back. 
//this function does that in a safe manner. 
vector<unsigned short> ACC::sendAndRead(unsigned int command, int buffsize)
{
	if(!checkUSB()) exit(EXIT_FAILURE);

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

//just read until data readout times out. 
//I have found that occassionally after a reboot
//of the boards or after a serious usb failure that
//the constant memory size buffers get over-filled and
//crash a raspberry pi or muddle the DAQ system. This 
//is an attempt to clear the line and have a fresh state. 
void ACC::emptyUsbLine()
{
	if(!checkUSB()) exit(EXIT_FAILURE);

	int send_counter = 0; //number of usb sends
	int max_sends = 10; //arbitrary. 
	bool loop_breaker = false; 
	unsigned int command = 0x1e0C0005; // a refreshing command
	int buffsize = 100000; //400kB of memory for safety
	vector<unsigned short> tempbuff;
	while(!loop_breaker)
	{
		usb->sendData(command);
		send_counter++;
		tempbuff = usb->safeReadData(buffsize + 2);

		//if it is exactly an ACC buffer size, success. 
		if(tempbuff.size() == 32)
		{
			loop_breaker = true;
		}
		if(send_counter > max_sends)
		{
			cout << "Something wrong with USB line, please reboot" << endl;
			loop_breaker = true;
		}
	}
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


//function that returns a pointer
//to the private ACC usb line. 
//currently used by the Config class
//to set configurations. 
stdUSB* ACC::getUsbStream()
{
	return usb;
}

//reads ACC info buffer only. short buffer
//that does not rely on any ACDCs to be connected. 
vector<unsigned short> ACC::readAccBuffer()
{
	if(!checkUSB()) exit(EXIT_FAILURE);

	//writing this tells the ACC to respond
	//with its own metadata
	unsigned int command = 0x1e0C0005; 
	//is OK to just pound the ACC with the
	//command until it responds. in a loop function "sendAndRead"
	vector<unsigned short> v_buffer = sendAndRead(command, CC_BUFFERSIZE);
	if(v_buffer.size() == 0)
	{
		cout << "USB comms to ACC are broken" << endl;
		cout << "(1) Turn off the ACC" << endl;
		cout << "(2) Unplug the USB cable power" << endl;
		cout << "(3) Turn on ACC" << endl;
		cout << "(4) Plug in USB" << endl;
		cout << "(5) Wait for green ACC LED to turn off" << endl;
		cout << "(5) Repeat if needed" << endl;
		cout << "Trying USB reset before closing... " << endl;
		usb->reset();
		sleep(1);
		exit(EXIT_FAILURE);
	}
	lastAccBuffer = v_buffer; //save as a private variable
	return v_buffer; //also return as an option
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
	//loads a ACC buffer into private member
	readAccBuffer(); 

	//parses the last acc buffer to 
	//see which ACDCs are aligned. 
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

	parsePedsAndConversions(); //load pedestals and LUT conversion factors onto ACDC objects.

}

//If the pedestals were set, and an ADC-counts to mV lineary scan
//was made for these boards, then there are autogenerated calibration
//files that should be loaded into private variables of the ACDC. 
//They are save to some directory as text files and stored for long-term
//use because the actual pedestal values and ADC-to-mv conversion
//for each sample will persist for longer than a single script function call.
//The directory and filenames are defines in the ACC.h. This function
//returns the number of board indices that do not have associated LIN
//files in the calibration directory. 
int ACC::parsePedsAndConversions()
{
	int linCounter = 0; //counts how many board numbers DO NOT have matching lin files
	int pedCounter = 0; //counts how many board numbers DO NOT have matching ped files
	double defaultConversion = 1200.0/4096.0; //default for ADC-to-mv conversion
	double defaultPed = 0.0; //sets the baseline around 600mV
	//loop over all connected boards
	for(ACDC* a: acdcs)
	{
		//peds and lins in ACDC objects are map
		//objects like map[chip] = vector[sample].
		//use this temp map as a container for parsed data.
		map<int, vector<double>> tempMap; 


		int bi = a->getBoardIndex();
		//look for the PED and LIN files for this index
		string pedfilename = string(CALIBRATION_DIRECTORY) + string(PED_TAG) + to_string(bi) + ".txt";
		string linfilename = string(CALIBRATION_DIRECTORY) + string(LIN_TAG) + to_string(bi) + ".txt";
		ifstream ifped(pedfilename);
		ifstream iflin(linfilename);
		
		//if the associated file does not exist
		//set default values defined above. 
		if(!(bool)ifped)
		{
			cout << "WARNING: Your board number " << bi << " does not have a pedestal calibration in the calibration folder: " << CALIBRATION_DIRECTORY << endl;
			pedCounter++;
			for(int channel = 1; channel <= a->getNumCh(); channel++)
			{
				vector<double> vtemp(a->getNumSamp(), defaultPed); //vector of 0's
				tempMap[channel] = vtemp;
			}
			a->setPeds(tempMap);
		}
		//otherwise, parse the file. 
		else
		{
			a->readPedsFromFile(ifped);
		}

		if(!(bool)iflin)
		{
			cout << "WARNING: Your board number " << bi << " does not have a linearity scan calibration in the calibration folder: " << CALIBRATION_DIRECTORY << endl;
			linCounter++;
			for(int channel = 1; channel <= a->getNumCh(); channel++)
			{
				vector<double> vtemp(a->getNumSamp(), defaultConversion); //vector of defaults
				tempMap[channel] = vtemp;
			}
			a->setConv(tempMap);
		}
		//otherwise, parse the file. 
		else
		{
			a->readConvsFromFile(iflin);
		}
		ifped.close();
		iflin.close();
	}

	return linCounter;
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
	if(pullNew ||lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	vector<int> connectedBoards;
	if(lastAccBuffer.size() < 3) 
	{
		cout << "Something wrong with ACC buffer" << endl;
		return connectedBoards;
	}

	//for explanation of the "2", see INFO1 of CC_STATE in packetUSB.vhd
	unsigned short alignment_packet = lastAccBuffer.at(2); 
	//binary representation of this packet is 1 if the
	//board is connected for both the first two bytes
	//and last two bytes. 
	
	for(int i = 0; i < MAX_NUM_BOARDS; i++)
	{
		//both (1<<i) and (1<<i+8) should be true if aligned & synced respectively
		if((alignment_packet & (1 << i)) && (alignment_packet & (1 << (i + 8))))
		{
			//the i'th board is connected

			//protection against a NASTY error I saw
			//due to a bad ACC buffer. We will need to investigate
			//but for now I am protecting against seg faults. 
			if(i > 3)
			{
				//return with no connected boards
				connectedBoards.clear();
				alignedAcdcIndices = connectedBoards;
				return connectedBoards;
			}

			connectedBoards.push_back(i);
		}
	}

	//this allows no vector clearing to be needed
	alignedAcdcIndices = connectedBoards;
	return connectedBoards;
}

//returns map[board][is ram full]
vector<int> ACC::checkFullRamRegisters(bool pullNew)
{
	if(pullNew ||lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	vector<int> tempRamFull;
	if(lastAccBuffer.size() < 5) 
	{
		cout << "Something wrong with ACC buffer" << endl;
		fullRam = tempRamFull;
		return tempRamFull;
	}

	unsigned short ram_packet = lastAccBuffer.at(4) & 0x000F;
	

	//this will change when the full 8 bits of 
	//xram_full in packetUSB.vhd are included in the ACC buffer. 
	int ramRegisters = 4; 
	for(int i = 0; i < ramRegisters; i++)
	{
		if((ram_packet & (1 << i)))
		{
			//the i'th board is connected
			tempRamFull.push_back(i);
		}
	}

	//this allows no map-clearing to be needed
	fullRam = tempRamFull; //private variable storage
	return tempRamFull; //return as an alternative. 

}

//returns map[board][is digitizing flag true]
vector<int> ACC::checkDigitizingFlag(bool pullNew)
{
	if(pullNew ||lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	//just in case the Acc doesnt have a good buffer
	vector<int> tempDigFlag;
	if(lastAccBuffer.size() < 5) 
	{
		cout << "Something wrong with ACC buffer" << endl;
		digFlag = tempDigFlag;
		return tempDigFlag;
	}

	unsigned short dig_packet = (lastAccBuffer.at(4) & 0x0F00) >> 8;
	
	//this will change when the full 8 bits of 
	//xram_full in packetUSB.vhd are included in the ACC buffer. 
	int bitsTransferred = 4; 
	for(int i = 0; i < bitsTransferred; i++)
	{
		if((dig_packet & (1 << i)))
		{
			//the i'th board is connected
			tempDigFlag.push_back(i);
		}
	}

	//this allows no map-clearing to be needed
	digFlag = tempDigFlag; //private variable storage
	return tempDigFlag; //return as an alternative. 
}

//returns map[board][is digitizing flag true]
//dcPkt indicates that an ACDC has started sending
//data to the ACC, but has not yet filled the ram. 
vector<int> ACC::checkDcPktFlag(bool pullNew)
{
	if(pullNew ||lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	//just in case the Acc doesnt have a good buffer
	vector<int> tempDcPktFlag;
	if(lastAccBuffer.size() < 5) 
	{
		cout << "Something wrong with ACC buffer" << endl;
		dcPkt = tempDcPktFlag;
		return tempDcPktFlag;
	}

	unsigned short dc_packet = (lastAccBuffer.at(4) & 0x00F0) >> 4;
	

	//this will change when the full 8 bits of 
	//xram_full in packetUSB.vhd are included in the ACC buffer. 
	int bitsTransferred = 4; 
	for(int i = 0; i < bitsTransferred; i++)
	{
		if((dc_packet & (1 << i)))
		{
			//the i'th board is connected
			tempDcPktFlag.push_back(i);
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
	if(pullNew ||lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	for(unsigned short val: lastAccBuffer)
	{
		printByte(val);
		cout << endl;
	}
	
}

//looks at the last ACC buffer and reads
//the last ACC event count. 
int ACC::getAccEventNumber(bool pullNew)
{
	if(pullNew || lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}


	//just in case the Acc doesnt have a good buffer
	if(lastAccBuffer.size() < 6) 
	{
		cout << "Something wrong with ACC buffer" << endl;
		return 0;
	}

	unsigned short evno_lo = lastAccBuffer.at(5);
	unsigned short evno_hi = lastAccBuffer.at(4) & (1 << 15); //one bit only..
	unsigned int evno = evno_lo + (evno_hi << 16);
	return (int)evno;
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

vector<int> ACC::unsignedShortToVector(unsigned short a)
{
	vector<int> result;
	int lenOfShort = 16;
	for(int i = 0; i < lenOfShort; i++)
	{
		if(a & (1 << i))
		{
			result.push_back(i);
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
	
	//default value for "boards" is empty. If so, then
	//software trigger all active boards from last
	//buffer query. 
	if(boards.size() == 0)
	{
		boards = alignedAcdcIndices;
	}

	cout << "Sending a software trigger to boards ";
	for(int bi: boards) cout << bi << ", ";
	cout << endl;

	//turn the board vector into a binary form
	//(0110), unsigned int mask. 
	unsigned short mask = vectorToUnsignedShort(boards);
	//the present version of firmware only allows
	//one to mask boards 0-3 (as opposed to 0-7). 
	//take this part out when moving to 8 board firmware. 
	mask = mask & 0x000F;

	//force the board to trigger on a certain 160MHz
	//clock cycle within the event. default is 0, and
	//cannot be more than 3. 
	bin = bin % 4;

	//send the command
	unsigned int command = 0x000E0000; 
	command = command | mask | (1 << 5) | (bin << 4); //currently not including bin-setting functionality

	usb->sendData(command);
}

//checks to see if there are any ACDC buffers
//in the ram of the ACC. If waitForAll = true (false by default),
//it will continue checking until all alignedAcdcs have sent
//data to the ACC RAM. Unfortunately, the CC event number doesnt
//increment in hardware trigger mode, so the evno is sent in 
//explicitly to keep data files consistent. 
//"raw" will subtract ped and convert from LUT calibration
//if set to false. 
//return codes:
//0 = data found and parsed successfully
//1 = data found but had a corrupt buffer
//2 = no data found
int ACC::readAcdcBuffers(bool waitForAll, int evno, bool raw)
{
	//First, loop and look for 
	//a fullRam flag on ACC indicating
	//that ACDCs have sent data to the ACC
	int maxChecks = 15; //will give up after this many
	int check = 0;
	bool pullNewAccBuffer = true;
	vector<int> boardsReadyForRead; //list of board indices that are ready to be read-out
	while(check < maxChecks)
	{
		//pull a new Acc buffer and parse
		//the data-ready state indicators. 
		checkDcPktFlag(pullNewAccBuffer);
		checkFullRamRegisters();

		//debug
		/*
		if(lastAccBuffer.size() > 4)
		{
			cout << "Ram/Pkt byte is: ";
			printByte(lastAccBuffer.at(4));
			cout << endl;
		}
		*/
	
		//check which ACDCs have both gotten a trigger
		//and have filled the ACC ram, thus starting
		//it's USB write flag. 
		unsigned short fr = vectorToUnsignedShort(fullRam);
		unsigned short dc = vectorToUnsignedShort(dcPkt);
		//a vector of indices that have both flags = 1
		boardsReadyForRead = unsignedShortToVector(fr & dc); 

		if(waitForAll)
		{
			if(boardsReadyForRead.size() != alignedAcdcIndices.size())
			{
				//have not gotten all ACDC data
				check++;
			}
			else
			{
				//found them all. 
				break;
			}
		}
		else
		{
			if(boardsReadyForRead.size() == 0)
			{
				//have not gotten any ACDC data
				check++;
			}
			else
			{
				//found at least 1 ACDC data
				break;
			}
		}
	}

	if(check == maxChecks)
	{
		cout << "ACDC buffers were never sent to the ACC" << endl;
		return 2;
	}

	//each ACDC needs to be queried individually
	//by the ACC for its buffer. 
	for(int bi: boardsReadyForRead)
	{
		unsigned int command = 0x1e0C0000; //base command for set readmode
		command = command | (unsigned int)(bi + 1); //which board to read

		//send 
		usb->sendData(command);
		//read only once. sometimes the buffer comes up empty. 
		//made a choice not to pound it with a loop until it
		//responds. 
		vector<unsigned short> acdc_buffer = usb->safeReadData(ACDC_BUFFERSIZE + 2);


		//----corrupt buffer checks begin
		//sometimes the ACDCs dont send back good
		//data. It is unclear why, but we would
		//just rather throw this event away. 
		bool corruptBuffer = false;
		if(acdc_buffer.size() == 0)
		{
			corruptBuffer = true;
		}
		int nonzerocount = 0;
		//if the first 20 bytes are all 0's, it is corrupt...
		for(int i = 0; i < (int)acdc_buffer.size() && i < 20; i++)
		{
			if(acdc_buffer[i] != (unsigned short)0) {nonzerocount++;}
		}
		if(nonzerocount == 0) {corruptBuffer = true;}

		if(corruptBuffer)
		{
			return 1;
		}
		//----corrupt buffer checks end. 


		//save this buffer a private member of ACDC
		//by looping through our acdc vector
		//and checking each index (not optimal but)
		//who cares about 4 loop iterations. 
		for(ACDC* a: acdcs)
		{
			if(a->getBoardIndex() == bi)
			{
				//set the buffer that we just read. 
				//There is a string of bool returning
				//functions that checks if the buffer is corrupt
				//in a sense that it doesn't follow the expected
				//packet order or packet format. Ultimately, this
				//is presently set by the Metadata.parseBuffer() member
				//and returns "bad buffer" if there are not NUM_PSEC 
				//number of info blocks. 
				corruptBuffer = !(a->setLastBuffer(acdc_buffer, evno)); //also triggers parsing function
				if(corruptBuffer)
				{
					cout << "********* Corrupt buffer caught at ACC level ****************" << endl;
					return 1;
				}
				//tells it explicitly to load the data
				//component of the buffer into private memory. 
				int retval;//to catch corrupt buffers
				retval = a->parseDataFromBuffer(raw); 
				if(retval == 1)
				{
					cout << "********* Corrupt buffer caught at ACDC parser level ****************" << endl;
					a->writeRawBufferToFile();
					return 1;
				}

			}
		}
	}


	return 0;
}


//identical to readAcdcBuffer but does an infinite
//loop when the trigMode is 1 (hardware trig) and
//switches toggles waitForAll depending on trig mode. 
//Unfortunately, the CC event number doesn't increment
//in hardware trigger mode, so that needs to be sent
//in explicitly via the logData function. 
//"raw" will subtract ped and convert from LUT calibration
//if set to false. 
//0 = data found and parsed successfully
//1 = data found but had a corrupt buffer
//2 = no data found
int ACC::listenForAcdcData(int trigMode, int evno, bool raw)
{


	bool waitForAll = false;
	bool pullNewAccBuffer = true;
	vector<int> boardsReadyForRead; //list of board indices that are ready to be read-out

	//this function is simply readAcdcBuffers
	//if the trigMode is software
	if(trigMode == 0)
	{
		waitForAll = true;
		int retval;
		//The ACC already sent a trigger, so
		//tell it not to send another during readout. 
		setAccTrigInvalid();
		retval = readAcdcBuffers(waitForAll, evno, raw);
		return retval;
	}

	//duration variables
	auto start = chrono::steady_clock::now(); //start of the current event listening. 
	auto now = chrono::steady_clock::now(); //just for initialization 
	auto printDuration = chrono::seconds(1); //prints as it loops and listens
	auto lastPrint = chrono::steady_clock::now();
	auto timeoutDuration = chrono::seconds(10); // will exit and reinitialize


	//setup a sigint capturer to safely
	//reset the boards if a ctrl-c signal is found
	struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);

	try
	{
		while(true)
		{
			now = chrono::steady_clock::now();
			if(chrono::duration_cast<chrono::seconds>(now - lastPrint) > printDuration)
			{
				cout << "Have been waiting for a trigger for " << chrono::duration_cast<chrono::seconds>(now - start).count() << " seconds" << endl;
				lastPrint = chrono::steady_clock::now();
			}
			if(chrono::duration_cast<chrono::seconds>(now - start) > timeoutDuration)
			{
				return 2;
			}

			//if sigint happens, 
			//return value of 3 tells
			//logger what to do. 
			if(quitacc.load())
			{
				return 3;
			}

			//throttle, without it the USB line becomes jarbled...
			//this is also the amount of time that the trigValid = 1
			//on the ACC, i.e. a window for events to happen. 
			usleep(10000); 

			//pull a new Acc buffer and parse
			//the data-ready state indicators. 
			checkDcPktFlag(pullNewAccBuffer);
			checkFullRamRegisters();

			//debug
			
			if(lastAccBuffer.size() > 4)
			{
				cout << "Ram/Pkt byte is: ";
				printByte(lastAccBuffer.at(4));
				cout << endl;
			}
		
			//check which ACDCs have both gotten a trigger
			//and have filled the ACC ram, thus starting
			//it's USB write flag. 
			
			//I want to see that dcPkt and fullRam
			//are equivalent. If one board finishes
			//sending data to ACC before another, for
			//example, we still want to wait. 
			std::sort(fullRam.begin(), fullRam.end());
			std::sort(dcPkt.begin(), dcPkt.end());
			if(dcPkt == fullRam && dcPkt.size() > 0)
			{
				//all boards have finished
				//sending data to ACC. 
				break;
			}

		}

		
		setAccTrigInvalid();
		//each ACDC needs to be queried individually
		//by the ACC for its buffer. 
		for(int bi: fullRam)
		{
			unsigned int command = 0x1e0C0000; //base command for set readmode
			command = command | (unsigned int)(bi + 1); //which board to read

			//send 
			usb->sendData(command);
			//read only once. sometimes the buffer comes up empty. 
			//made a choice not to pound it with a loop until it
			//responds. 
			vector<unsigned short> acdc_buffer = usb->safeReadData(ACDC_BUFFERSIZE + 2);


			//----corrupt buffer checks begin
			//sometimes the ACDCs dont send back good
			//data. It is unclear why, but we would
			//just rather throw this event away. 
			bool corruptBuffer = false;
			if(acdc_buffer.size() == 0)
			{
				corruptBuffer = true;
			}
			int nonzerocount = 0;
			//if the first 20 bytes are all 0's, it is corrupt...
			for(int i = 0; i < (int)acdc_buffer.size() && i < 20; i++)
			{
				if(acdc_buffer[i] != (unsigned short)0) {nonzerocount++;}
			}
			if(nonzerocount == 0) {corruptBuffer = true;}

			if(corruptBuffer)
			{
				return 1;
			}
			//----corrupt buffer checks end. 


			//save this buffer a private member of ACDC
			//by looping through our acdc vector
			//and checking each index 
			for(ACDC* a: acdcs)
			{
				if(a->getBoardIndex() == bi)
				{
					//set the buffer that we just read. 
					//There is a string of bool returning
					//functions that checks if the buffer is corrupt
					//in a sense that it doesn't follow the expected
					//packet order or packet format. Ultimately, this
					//is presently set by the Metadata.parseBuffer() member
					//and returns "bad buffer" if there are not NUM_PSEC 
					//number of info blocks. 
					corruptBuffer = !(a->setLastBuffer(acdc_buffer, evno)); //also triggers parsing function
					if(corruptBuffer)
					{
						cout << "********* got a corrupt buffer (1) ****************" << endl;
						return 1;
					}
					//tells it explicitly to load the data
					//component of the buffer into private memory. 
					corruptBuffer = a->parseDataFromBuffer(raw); 
					if(corruptBuffer)
					{
						cout << "********* got a corrupt buffer (2) ****************" << endl;
						return 1;
					}
				}
			}
		}
	}
	catch(string mechanism)
	{
		cout << mechanism << endl;
		return 2;
	}
	
	return 0;
}

//this is a function that sends a specific set
//of usb commands to configure the boards for
//triggering on real events. 
void ACC::initializeForDataReadout(int trigMode)
{
	//software trigger
	if(trigMode == 0)
	{
		//i'm taking this directly from a USB listening
		//mode of the old software. it may not be unique.
		//it is cryptic and is possibly unstable. 
		setFreshReadmode();
		setAccTrigInvalid();
		setFreshReadmode();
		resetAccTrigger();

		prepSync();
		softwareTrigger();
		makeSync();
	}
	//hardware trigger
	else if(trigMode == 1)
	{
		setFreshReadmode();
		setAccTrigInvalid();
		setFreshReadmode();
		resetAccTrigger();

		setAccTrigInvalid();

		prepSync();
		setAccTrigValid();
		makeSync();
	}
	
	//other trigger modes soon to come
	return;
}

//a set of specific usb commands to reset
//the board configurations to a well defined
//state after doing a logData data collection.
void ACC::dataCollectionCleanup(int trigMode)
{
	//i'm taking this directly from a USB listening
	//mode of the old software. it may not be unique.
	//it is cryptic and is possibly unstable.
	if(trigMode == 0)
	{
		setAccTrigInvalid(); //b4
		resetAccTrigger(); //b1
		resetAcdcTrigger();
		setFreshReadmode(); //c0

		setAccTrigInvalid(); //b4
		resetAccTrigger(); //b1
		resetAcdcTrigger();
		setFreshReadmode(); //c0
	}

	else if(trigMode == 1)
	{
		setAccTrigInvalid(); //b4
		resetAccTrigger(); //b1
		resetAcdcTrigger(); //c010
		setFreshReadmode(); //c0

		setAccTrigInvalid(); //b4
		resetAccTrigger(); //b1
		resetAcdcTrigger();
		setFreshReadmode(); //c0
	}
	

}


//tells ACDCs to clear their ram. 
//necessary when closing the program, for example.
void ACC::dumpData()
{
	for(ACDC* a: acdcs)
	{
		//prep the ACC
		resetAccTrigger();
		resetAccTrigger();
		int bi = a->getBoardIndex();
		unsigned int command = 0x1e0C0000; //base command for set readmode
		command = command | (unsigned int)(bi + 1); //which board to read

		//send and read. 
		usb->sendData(command);
		usb->safeReadData(ACDC_BUFFERSIZE + 2);

	}

}


//just tells each ACDC that has a fullRam flag
// to write its data and metadata maps to the filestreams. 
//Event number is passed to the ACDC objects to help. 
void ACC::writeAcdcDataToFile(ofstream& d, ofstream& m)
{
	for(int bi: fullRam)
	{
		for(ACDC* a: acdcs)
		{
			if(a->getBoardIndex() == bi)
			{
				a->writeDataToFile(d, m);
			}
		}
	}
}



void ACC::testFunction()
{

	unsigned int command;
	command = 0x1e0a0001;
	usb->sendData(command);
	sleep(3);
	command = 0x1e0a0000;
	usb->sendData(command);
	sleep(3);
	command = 0x1e0a0001;
	usb->sendData(command);
	sleep(3);
}




//short circuits the Config - class based
//pedestal setting procedure. This is primarily
//used for calibration functions. Ped is in ADC counts
//from 0 to 4096. If boards is empty, will do to all connected
bool ACC::setPedestals(unsigned int ped, vector<int> boards)
{
	//default is empty vector, so do ped setting to all boards
	if(boards.size() == 0)
	{
		boards = alignedAcdcIndices;
	}

	//loop over connected boards
	int bi;
	int numChips;
	unsigned int command, tempWord, chipAddress;
	bool failCheck;
	for(ACDC* a: acdcs)
	{
		bi = a->getBoardIndex();
		//if this isn't in the selected board list, 
		//dont try to set a pedestal. 
		if(std::find(boards.begin(), boards.end(), bi) == boards.end())
		{
			continue;
		}

		numChips = a->getNumPsec();
		//set pedestals and thresholds
		for(int chip = 0; chip < numChips; chip++)
		{
			chipAddress = (1 << chip) << 20; //20 magic number

			//pedestal
			command = 0x00030000; //ped setting command. 
			tempWord = ped | (bi << 25) | chipAddress; //magic number 25. 
			command = command | tempWord;
			failCheck = usb->sendData(command); //set ped on that chip
			if(!failCheck)
			{
				cout << "Failed setting pedestal on board " << bi << " chip " << chip << endl;
				return false;
			}
		}
	}

	
	return true;
}


//-----------This class of functions are short usb
//-----------commands that don't have a great comms
//-----------structure. Giving them a name and their own
//-----------function helps to organize the code. 

//(1) clearing the Acc instruction in firmware
//(2) sets Acc trigger = valid
//(3) CC_SYNC goes to 1 which makes the
//firmware wait 50,000 clocks before sending
//the usb message. 
void ACC::prepSync()
{
	unsigned int command = 0x1e0B0018;
	usb->sendData(command);
}

//(1) clearing the Acc instruction in firmware
//(2) sets Acc trigger = valid
//(3) CC_SYNC goes to 0 which means it
//will send the usb message immediately
void ACC::makeSync()
{
	unsigned int command = 0x1e0B0010;
	usb->sendData(command);
}

//(1) resets firmware in trigger and time
//(2) does software reset of transeivers.vhd
//(3) sets CC_INSTRUCTION to the command. 
//Previously called "manage_cc_fifo", but now
//calling it "resetAccTrigger". The software
//reset in transeivers flags that software is
//done reading over usb and goes to LVDS idle. 
//Also sets SOFT_TRIG to 0 if it was at 1. Also
//sets TRIG_OUT signal to 0. 
void ACC::resetAccTrigger()
{
	unsigned int command = 0x1e0B0001;
	usb->sendData(command);
}

//this command does a number of things
//and the name may not be appropriate. 
//if you think of a better name, please
//feel free to change it. 
void ACC::resetAcdcTrigger()
{
	unsigned int command = 0x1e0c0010; //set trig src 0. 
	usb->sendData(command);
}


//there is an option to use one of
//the front end boards (ACDCs) as
//a hardware trigger source. It is
//set via the 1e0c register (somewhat bad
//design i think). 
//int src:
//0=ext, 3 = board0, 4=b1, 6=b2, 7=b3
void ACC::setHardwareTrigSrc(int src)
{
	unsigned int byteSuffix;
	byteSuffix = (1 << 3) | (1 << 4) | ((unsigned short)src << 13);
	unsigned int command = 0x1e0c0000; 
	command = command | byteSuffix; 
	usb->sendData(command);
}

//one flag that is required for
//a signal from the ACC to 
//be sent to trigger the ACDC
void ACC::setAccTrigValid()
{
	unsigned int command = 0x1e0B0006;
	usb->sendData(command);
}

void ACC::setAccTrigInvalid()
{
	unsigned int command = 0x1e0B0004;
	usb->sendData(command);
}

void ACC::setFreshReadmode()
{
	unsigned int command = 0x1e0C0000;
	usb->sendData(command);
}

