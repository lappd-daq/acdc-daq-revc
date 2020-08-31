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
}


ACC::~ACC()
{
	cout << "Calling acc destructor" << endl;
	clearAcdcs();
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
	if(!checkUSB())
	{
		cout << "USB line is not functioning" << endl;
		vector<unsigned short> blank;
		lastAccBuffer = blank; //ACC local variable
		return blank;
	}

	//writing this tells the ACC to respond
	//with its own metadata
	unsigned int command = 0x000C0005; 
	usb->sendData(command);

	vector<unsigned short> v_buffer = usb->safeReadData(CC_BUFFERSIZE);
	if(v_buffer.size() == 0)
	{
		cout << "Received no ACC info buffer upon request" << endl;
		vector<unsigned short> blank;
		return blank;
	}
	lastAccBuffer = v_buffer; //ACC local variable
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
//retval: 
//1 if there are ACDCs connected
//0 if none. 
int ACC::createAcdcs()
{
	//update link status, then read the ACC info buffer.
	whichAcdcsConnected(); 

	//if there are no ACDCs, return 0
	if(alignedAcdcIndices.size() == 0)
	{
		return 0;
	}
	
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

	return 1;
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




vector<int> ACC::whichAcdcsConnected()
{

	//update the link status of the ACC, i.e. check boards
	//by sending a message down the LVDS lines and looking for a reply.
	updateLinkStatus();

	//read the updated info buffer
	readAccBuffer();
	

	vector<int> connectedBoards;
	if(lastAccBuffer.size() < 3) 
	{
		cout << "Something wrong with ACC buffer" << endl;
		return connectedBoards;
	}

	unsigned short alignment_packet = lastAccBuffer.at(2); //the 3rd word
	//binary representation of this packet is 1 if the
	//board is connected for both the first two bytes
	//and last two bytes. 
	
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


//in the ACC local info buffer (0x000D0000), there is a word
//that says both (1) whether each acdc has recently started transferring
//data up the ethernet cables and (2) whether that transfer has finished.
vector<int> ACC::acdcsTransferringData(bool pullNew)
{
	if(pullNew ||lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	//just in case the Acc doesnt have a good buffer
	vector<int> whichBoardsTransferring;
	if(lastAccBuffer.size() < 5) 
	{
		cout << "Something wrong with ACC buffer" << endl;
		boardsTransferring = whichBoardsTransferring; //ACC local variable
		return whichBoardsTransferring;
	}

	unsigned short transferWord = (lastAccBuffer.at(4) & 0xF0) >> 4;
	

	cout << "Transfering now word is: " << transferWord << endl;
	for(int bo = 0; bo < MAX_NUM_BOARDS; bo++)
	{
		if((transferWord & (1 << bo)))
		{
			//the bo'th board is connected
			whichBoardsTransferring.push_back(bo);
		}
	}

	boardsTransferring = whichBoardsTransferring; //ACC local variable
	return whichBoardsTransferring; //return as an alternative. 
}

//in the ACC local info buffer (0x000D0000), there is a word
//that says both (1) whether each acdc has recently started transferring
//data up the ethernet cables and (2) whether that transfer has finished.
vector<int> ACC::acdcsDoneTransferringData(bool pullNew)
{
	if(pullNew ||lastAccBuffer.size() == 0)
	{
		readAccBuffer();
	}

	//just in case the Acc doesnt have a good buffer
	vector<int> whichBoardsDone;
	if(lastAccBuffer.size() < 5) 
	{
		cout << "Something wrong with ACC buffer" << endl;
		boardsDoneTransferring = whichBoardsDone; //ACC local variable
		return whichBoardsDone;
	}

	unsigned short transferWord = (lastAccBuffer.at(4) & 0xF);
	cout << "Transfering done word is: " << transferWord << endl;
	for(int bo = 0; bo < MAX_NUM_BOARDS; bo++)
	{
		if((transferWord & (1 << bo)))
		{
			//the bo'th board is connected
			whichBoardsDone.push_back(bo);
		}
	}

	boardsDoneTransferring = whichBoardsDone; //ACC local variable
	return whichBoardsDone; //return as an alternative. 
}


//looks at the last ACC buffer and reads
//the last ACC event count. 
int ACC::getAccEventNumber(bool pullNew)
{
	//if you are trying to get a new buffer 
	//(incrementing the acc event number)
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

	unsigned short evno_lo = lastAccBuffer.at(6);
	unsigned short evno_hi = lastAccBuffer.at(5);
	unsigned int evno = evno_lo + (evno_hi << 16);
	return (int)evno;
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
void ACC::softwareTrigger(vector<int> boards)
{
	
	//default value for "boards" is empty. If so, then
	//software trigger all active boards from last
	//buffer query. 
	if(boards.size() == 0)
	{
		boards = alignedAcdcIndices;
	}

	//turn the board vector into a binary form
	//(0110), unsigned int mask. 
	unsigned int mask = vectorToUnsignedInt(boards);


	//send the command
	unsigned int command = 0xFF0E0000 | mask; 
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
int ACC::readAcdcBuffers(int evno, bool parse)
{
	//First, loop and look for 
	//a fullRam flag on ACC indicating
	//that ACDCs have sent data to the ACC
	int maxChecks = 1000; //will wait for LVDS transfer to finish over this many loops
	int check = 0;
	bool pullNewAccBuffer = true;
	vector<int> boardsReadyForRead; //list of board indices that are ready to be read-out
	while(check < maxChecks)
	{
		
		//pull a new Acc buffer and parse
		//the data-ready state indicators. 
		acdcsTransferringData(pullNewAccBuffer);
		acdcsDoneTransferringData();

	
		//check which ACDCs have both gotten a trigger
		//and have filled the ACC ram, thus starting
		//it's USB write flag. 
		unsigned short tr = vectorToUnsignedShort(boardsTransferring);
		unsigned short dtr = vectorToUnsignedShort(boardsDoneTransferring);

		//if the boards that have started transmitting data
		//have finished transmitting data. 
		if(tr == dtr)
		{
			boardsReadyForRead = unsignedShortToVector(tr); 
			break;
		}


		if(check >= maxChecks)
		{
			break;
		}
		check++;	
	}

	if(check == maxChecks)
	{
		cout << "ACDC buffers were never sent to the ACC" << endl;
		resetAccRamFlags();
		return 2;
	}


	//each ACDC needs to be queried individually
	//by the ACC for its buffer. 
	int corruptBufferCount = 0;
	for(int bi: boardsReadyForRead)
	{
		unsigned int command = 0x000C0000; //base command for set readmode
		command = command | (unsigned int)(bi + 1); //which board to read

		//send 
		usb->sendData(command);
		usleep(6000); //usb takes this long to transfer from ACC ram to computer. 

		vector<unsigned short> acdc_buffer = usb->safeReadData(ACDC_BUFFERSIZE);
		//just throw it away, we will repeat. 
		if(parse)
		{
			for(ACDC* a: acdcs)
			{
				if(a->getBoardIndex() == bi)
				{
					int error_code = a->parseDataFromBuffer(acdc_buffer, evno); //0 is event number...
					if(error_code != 0)
					{
						cout << "Got corrupt buffer of type " << error_code << " from board " << bi << endl;
						corruptBufferCount++;
					}
				}
			}
		}
	}

	//resetAccRamFlags();
	return corruptBufferCount;
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
int ACC::listenForAcdcData(int trigMode, int evno)
{
	bool pullNewAccBuffer = true;
	vector<int> boardsReadyForRead; //list of board indices that are ready to be read-out

	//this function is simply readAcdcBuffers
	//if the trigMode is software
	if(trigMode == 0)
	{
		int retval;
		//The ACC already sent a trigger, so
		//tell it not to send another during readout. 
		setAccTrigInvalid();
		retval = readAcdcBuffers(evno);
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
			usleep(100000); 


			//pull a new Acc buffer and parse
			//the data-ready state indicators. 
			acdcsTransferringData(pullNewAccBuffer);
			acdcsDoneTransferringData();

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
			
			//If one board finishes
			//sending data to ACC before another, for
			//example, we still want to wait. 
			std::sort(boardsTransferring.begin(), boardsTransferring.end());
			std::sort(boardsDoneTransferring.begin(), boardsDoneTransferring.end());
			if(boardsTransferring == boardsDoneTransferring && boardsDoneTransferring.size() > 0)
			{
				//all boards have finished
				//sending data to ACC. 
				break;
			}

		}

		
		setAccTrigInvalid();
		//each ACDC needs to be queried individually
		//by the ACC for its buffer. 
		for(int bi: boardsDoneTransferring)
		{
			unsigned int command = 0x000C0000; //base command for set readmode
			command = command | (unsigned int)(bi + 1); //which board to read

			//send 
			usb->sendData(command);
			usleep(6000);

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
					int retval = a->parseDataFromBuffer(acdc_buffer, evno); //also triggers parsing function
					if(retval != 0) corruptBuffer = true;
					if(corruptBuffer)
					{
						cout << "********* got a corrupt buffer with retval " << retval << " ****************" << endl;
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
		resetAccRamFlags();

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
		resetAccRamFlags();

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
	return;
}


//tells ACDCs to clear their ram. 
//necessary when closing the program, for example.
void ACC::dumpData()
{
	for(ACDC* a: acdcs)
	{
		//prep the ACC
		resetAccRamFlags();
		resetAccRamFlags();
		int bi = a->getBoardIndex();
		unsigned int command = 0x1e0C0000; //base command for set readmode
		command = command | (unsigned int)(bi + 1); //which board to read

		//send and read. 
		usb->sendData(command);
		usb->safeReadData(ACDC_BUFFERSIZE + 2);

	}

}



int ACC::testFunction()
{

	int corruptBufferCount = 0;


	softwareTrigger();

	//First, loop and look for 
	//a fullRam flag on ACC indicating
	//that ACDCs have sent data to the ACC
	int maxChecks = 1000; //will wait for LVDS transfer to finish over this many loops
	int check = 0;
	bool pullNewAccBuffer = true;
	vector<int> boardsReadyForRead; //list of board indices that are ready to be read-out
	while(check < maxChecks)
	{
		
		//pull a new Acc buffer and parse
		//the data-ready state indicators. 
		acdcsTransferringData(pullNewAccBuffer);
		acdcsDoneTransferringData();

	
		//check which ACDCs have both gotten a trigger
		//and have filled the ACC ram, thus starting
		//it's USB write flag. 
		unsigned short tr = vectorToUnsignedShort(boardsTransferring);
		unsigned short dtr = vectorToUnsignedShort(boardsDoneTransferring);

		//if no boards have started transmitting ACDC data,
		//then no need to continue. 
		if(tr == 0)
		{
			cout << "Tried reading ACDC data but none had been told to send data" << endl;
			cout << "Try sending a trigger first." << endl;
			break;
		}

		//if the boards that have started transmitting data
		//have finished transmitting data. 
		if(tr == dtr)
		{
			boardsReadyForRead = unsignedShortToVector(tr); 
			break;
		}


		if(check >= maxChecks)
		{
			break;
		}
		check++;	
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
		unsigned int command = 0x000C0000; //base command for set readmode
		command = command | (unsigned int)(bi + 1); //which board to read

		//send 
		usb->sendData(command);
		usleep(6000); 

		vector<unsigned short> acdc_buffer = usb->safeReadData(ACDC_BUFFERSIZE);
		//just throw it away, we will repeat. 
		
		for(ACDC* a: acdcs)
		{
			if(a->getBoardIndex() == bi)
			{
				int error_code = a->parseDataFromBuffer(acdc_buffer, 0); //0 is event number...
				if(error_code != 0)
				{
					cout << "Got corrupt buffer of type " << error_code << " from board " << bi << endl;
					corruptBufferCount++;
				}
				
				/*
				//uncomment if you would like to print the buffer to file
				int count = 0;
				ofstream tempof("output", ios_base::trunc);
				for(unsigned short k: acdc_buffer)
				{
					tempof << k << ", "; //decimal
					stringstream ss;
					ss << std::hex << k;          
					string hexstr(ss.str());
					tempof << hexstr << ", "; //hex
					unsigned n;
					ss >> n;
					bitset<16> b(n);
					tempof << b.to_string(); //binary
					tempof << "   " << count << "th byte" << endl;
					count++;
				}

				cout << "size of acdc_buffer is " << acdc_buffer.size() << endl;
				*/
			}
		}
		
	}



	return corruptBufferCount;
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

//toggles the calibration input line switches that the
//ACDCs have on board. 0xFF selects all boards for this toggle. 
//Similar for channel mask, except channels are ganged in pairs of
//2 for hardware reasons. So 0x0001 is channels 1 and 2 enabled. 
void ACC::toggleCal(int onoff, unsigned int boardmask, unsigned int channelmask)
{
	unsigned int command = 0x00020000;
	unsigned int boardAddress = boardmask;


	boardAddress = boardAddress & 0xF; 

	command = command | (boardAddress << 24);

	//the firmware just uses the channel mask to toggle
	//switch lines. So if the cal is off, all channel lines
	//are set to be off. Else, uses channel mask
	if(onoff == 1)
	{
		command = command | channelmask;
	}

	usb->sendData(command);

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
	unsigned int command = 0xFF0B0010;
	usb->sendData(command);
}

//This function:
//resets the boardsTransferring and boardsDoneTransferring bits on ACC
//resets the ACC-to-ACDC trigger lines to 0
void ACC::resetAccRamFlags()
{
	unsigned int command = 0xFF0B0001;
	usb->sendData(command);
}

//one flag that is required for
//a signal from the ACC to 
//be sent to trigger the ACDC
void ACC::setAccTrigValid()
{
	unsigned int command = 0xFF0B0006;
	usb->sendData(command);
}

void ACC::setAccTrigInvalid()
{
	unsigned int command = 0xFF0B0004;
	usb->sendData(command);
}

void ACC::setFreshReadmode()
{
	unsigned int command = 0xFF0C0000;
	usb->sendData(command);
}


//------------reset functions

//the lightest reset. does not
//try to realign LVDS, does not try
//to reset ACDCs, just tries to wake the
//USB chip. 
void ACC::usbWakeup()
{
	unsigned int command = 0xFF040EFF;
	usb->sendData(command);
}

//This is sent down to the ACDCs
//and has them individually reset their
//timestamps, dll, self-trigger, etc. 
void ACC::resetACDCs()
{
	unsigned int command = 0xFF04F000;
	usb->sendData(command);
}

//reset ACDCs and realign, closest thing to power cycle
void ACC::hardReset()
{
	unsigned int command = 0xFF040FFF;
	usb->sendData(command);
}

void ACC::updateLinkStatus()
{
	unsigned int command = 0xFF050000;
	usb->sendData(command);
	//now the ACDC will automatically send its entire
	//data buffer size, full of zeros. It fills the ACC
	//ram, which stays filled until we request the dummy
	//data back. So, do a readAcdc but with no data parsing. 
	readAcdcBuffers(0, false); //0 is event number, false is no data parsing.
}


//-----------end reset functions


//----------auxhilliary printing functions--------//
//auxhiliary function for debugging
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


void ACC::printRawAccBuffer()
{
	readAccBuffer();
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


//just tells each ACDC that has finished transferring data
// to write its data and metadata maps to the filestreams. 
//Event number is passed to the ACDC objects to help. 
void ACC::writeAcdcDataToFile(ofstream& d, ofstream& m)
{
	for(int bi: boardsDoneTransferring)
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
//---------end aux functions ---------------/

