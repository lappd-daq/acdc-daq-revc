#include "ACC_ETH.h" 

// >>>> ID:0 Sigint handling
static std::atomic<bool> quitacc(false); 
void ACC_ETH::got_signal(int){quitacc.store(true);}

//------------------------------------------------------------------------------------//
//--------------------------------Constructor/Deconstructor---------------------------//

// >>>> ID:1 Constructor
ACC_ETH::ACC_ETH()
{
    eth = new Ethernet("127.0.0.1","5000");
    std:cout << "Connect to: " << "127.0.0.1" << ":" << "5000" << std::endl;
}

// >>>> ID:2 Constructor with IP and port arguments
ACC_ETH::ACC_ETH(std::string ip, std::string port)
{
    eth = new Ethernet(ip,port);
    std:cout << "Connect to: " << ip << ":" << port << std::endl;
}

// >>>> ID:3 Destructor 
ACC_ETH::~ACC_ETH()
{
	eth->CloseInterface();
    delete eth;
    eth = 0;
    std::cout << "ACC destructed" << std::endl;
}

//------------------------------------------------------------------------------------//
//---------------------------Setup functions for ACC/ACDC-----------------------------//

// >>>> ID 4: Main init function that controls generalk setup as well as trigger settings
int ACC_ETH::InitializeForDataReadout(unsigned int boardmask, int triggersource)
{
    if(triggersource<0 || triggersource>9)
    {
        std::cout << "Invalid trigger source chosen, please choose between and including 0 and 9" << std::endl;
        return -1;
    }

    //Get the connected ACDCs
    command_address  = CML.ACDC_Board_Detect;
    command_value = 0;

    uint64_t DetectedBoards = eth->ReceiveDataSingle(command_address,command_value);

    if(DetectedBoards==0){return -2;}

    for(int bi=0; bi<MAX_NUM_BOARDS; bi++)
    {
        if(DetectedBoards & (1<<bi))
        {
            alignedAcdcIndices.push_back(bi);
        }
    }

    std::cout << "Connected boards: " << alignedAcdcIndices.size() << " at ACC-port | ";
    for(int k: alignedAcdcIndices)
    {
        std::cout << k << " | ";
    }
    std::cout << std::endl;

	// Set trigger conditions
    bool ret;
    SetTriggerSource(boardmask,triggersource);

    command_address = CML.SMA_Polarity_Select;
    command_value = ACC_sign;
    ret = eth->SendData(command_address,command_value,"w");
    if(!ret){printf("Could not send command 0x%08llX with value %i to set ACC SMA sign!\n",command_address,command_value);}

    command_address = CML.ACDC_Command;
    command_value = CML.SMA_Polarity_Select | (boardmask<<24) | ACDC_sign; //????
    ret = eth->SendData(command_address,command_value,"w");
    if(!ret){printf("Could not send command 0x%08llX with value %i to set ACDC SMA sign!\n",command_address,command_value);}

    command_address = CML.Beamgate_Window_Start;
    command_value = validation_start;
    ret = eth->SendData(command_address,command_value,"w");
    if(!ret){printf("Could not send command 0x%08llX with value %i to set Beamgate window start!\n",command_address,command_value);}

    command_address = CML.Beamgate_Window_Length;
    command_value = validation_window;
    ret = eth->SendData(command_address,command_value,"w");
    if(!ret){printf("Could not send command 0x%08llX with value %i to set Beamgate window length!\n",command_address,command_value);}

    command_address = CML.PPS_Beamgate_Multiplex;
    command_value = PPSBeamMultiplexer;
    ret = eth->SendData(command_address,command_value,"w");
    if(!ret){printf("Could not send command 0x%08llX with value %i to set Beamgate/PPS Multiplexer!\n",command_address,command_value);}

    command_address = CML.PPS_Divide_Ratio;
    command_value = PPSRatio;
    ret = eth->SendData(command_address,command_value,"w");
    if(!ret){printf("Could not send command 0x%08llX with value %i to set PPS Multiplier!\n",command_address,command_value);}

	return 0;
}

// >>>> ID 5: Set up the trigger source
int ACC_ETH::SetTriggerSource(unsigned int boardmask, int triggersource)
{	
    for(unsigned int i=0; i<MAX_NUM_BOARDS; i++)
    {
        if(boardmask & (1<<i))
        {
            command_address = CML.Trigger_Mode_Select | i; 
        }else
        {
            continue;
        }
        command_value = triggersource;

        bool ret = eth->SendData(command_address,command_value,"w");
        if(!ret){printf("Could not send command 0x%08llX with value %i to set trigger source!\n",command_address,command_value);}
    }
    return 0;
}

//------------------------------------------------------------------------------------//
//---------------------------Read functions listening for data------------------------//

/*ID 6: Main listen fuction for data readout. Runs for 5s before retuning a negative*/
int ACC_ETH::listenForAcdcData(int trigMode, bool raw, string timestamp)
{
	// bool corruptBuffer;
	// vector<int> boardsReadyForRead;
	// map<int,int> readoutSize;
	// unsigned int command; 
	// bool clearCheck;

	// //filename logistics
	// string outfilename = "./Results/";
	// string datafn;
	// ofstream dataofs;

	// //this function is simply readAcdcBuffers
	// //if the trigMode is software
	// //if(trigMode == 1)
	// //{
	// //	int retval = readAcdcBuffers(raw, timestamp);
	// //	return retval;
	// //}

	// //setup a sigint capturer to safely
	// //reset the boards if a ctrl-c signal is found
	// struct sigaction sa;
	// memset( &sa, 0, sizeof(sa) );
	// sa.sa_handler = got_signal;
	// sigfillset(&sa.sa_mask);
	// sigaction(SIGINT,&sa,NULL);

	// //Enables the transfer of data from ACDC to ACC
   	// enableTransfer(1); 
  	
	// //duration variables
	// auto start = chrono::steady_clock::now(); //start of the current event listening. 
	// auto now = chrono::steady_clock::now(); //just for initialization 
	// auto printDuration = chrono::seconds(2); //prints as it loops and listens
	// auto lastPrint = chrono::steady_clock::now();
	// auto timeoutDuration = chrono::seconds(20); // will exit and reinitialize

	// while(true)
	// { 
	// 	//Clear the boards read vector
	// 	boardsReadyForRead.clear(); 
	// 	readoutSize.clear();
		
	// 	//Time the listen fuction
	// 	now = chrono::steady_clock::now();
	// 	if(chrono::duration_cast<chrono::seconds>(now - lastPrint) > printDuration)
	// 	{	
	// 		string err_msg = "Have been waiting for a trigger for ";
	// 		err_msg += to_string(chrono::duration_cast<chrono::seconds>(now - start).count());
	// 		err_msg += " seconds";
	// 		writeErrorLog(err_msg);
	// 		for(int i=0; i<MAX_NUM_BOARDS; i++)
	// 		{
	// 			string err_msg = "Buffer for board ";
	// 			err_msg += to_string(i);
	// 			err_msg += " has ";
	// 			err_msg += to_string(lastAccBuffer.at(16+i));
	// 			err_msg += " words";
	// 			writeErrorLog(err_msg);
	// 		}
	// 		lastPrint = chrono::steady_clock::now();
	// 	}
	// 	if(chrono::duration_cast<chrono::seconds>(now - start) > timeoutDuration)
	// 	{
	// 		return 2;
	// 	}

	// 	//If sigint happens, return value of 3
	// 	if(quitacc.load())
	// 	{
	// 		return 3;
	// 	}

	// 	//Request the ACC info frame to check buffers
	// 	command = 0x00200000;
	// 	usbcheck=usb->sendData(command);
	// 	if(usbcheck==false)
	// 	{
	// 		writeErrorLog("Send Error");
	// 		clearCheck = emptyUsbLine();
	// 		if(clearCheck==false)
	// 		{
	// 			writeErrorLog("After failed send, emptying the USB lines failed as well");
	// 		}
	// 	}
		
	// 	lastAccBuffer = usb->safeReadData(ACCFRAME);
		
	// 	if(lastAccBuffer.size()==0)
	// 	{
	// 		std::cout << "ACCFRAME came up with " << lastAccBuffer.size() << std::endl;
	// 		continue;
	// 	}

	// 	//go through all boards on the acc info frame and if 7795 words were transfered note that board
	// 	for(int k=0; k<MAX_NUM_BOARDS; k++)
	// 	{
	// 		if(lastAccBuffer.at(14) & (1 << k))
	// 		{
	// 			if(lastAccBuffer.at(16+k)==PSECFRAME)
	// 			{
	// 				boardsReadyForRead.push_back(k);
	// 				readoutSize[k] = PSECFRAME;
	// 			}else if(lastAccBuffer.at(16+k)==PPSFRAME)
	// 			{
	// 				boardsReadyForRead.push_back(k);
	// 				readoutSize[k] = PPSFRAME;
	// 			}
	// 		}
	// 	}

	// 	//old trigger
	// 	if(boardsReadyForRead==alignedAcdcIndices)
	// 	{
	// 		break;
	// 	}

	// 	/*new trigger
	// 	std::sort(boardsReadyForRead.begin(), boardsReadyForRead.end());
	// 	bool control = false;
	// 	if(boardsReadyForRead.size()%2==0)
	// 	{
	// 		for(int m=0; m<boardsReadyForRead.size(); m+=2)
	// 		{
	// 			if({boardsReadyForRead[m],boardsReadyForRead[m+1]}=={0,1})
	// 			{
	// 				control = true;
	// 			}else if({boardsReadyForRead[m],boardsReadyForRead[m+1]}=={2,3})
	// 			{
	// 				control = true;
	// 			}else if({boardsReadyForRead[m],boardsReadyForRead[m+1]}=={4,5})
	// 			{
	// 				control = true;
	// 			}else if({boardsReadyForRead[m],boardsReadyForRead[m+1]}=={6,7})
	// 			{
	// 				control = true;
	// 			}else
	// 			{
	// 				control = false;
	// 			}
	// 		}
	// 		if(control==true)
	// 		{
	// 			break;
	// 		}
	// 	}*/
	// }

	// //each ACDC needs to be queried individually
	// //by the ACC for its buffer. 
	// for(int bi: boardsReadyForRead)
	// {
	// 	//base command for set readmode and which board bi to read
	// 	unsigned int command = 0x00210000; 
	// 	command = command | (unsigned int)(bi); 
	// 	usbcheck=usb->sendData(command); if(usbcheck==false){writeErrorLog("Send Error");}	

	// 	//Tranfser the data to a receive vector
	// 	vector<unsigned short> acdc_buffer = usb->safeReadData(readoutSize[bi]);

	// 	//Handles buffers =/= 7795 words
	// 	if((int)acdc_buffer.size() != readoutSize[bi])
	// 	{
	// 		string err_msg = "Couldn't read 7795 words as expected! Tryingto fix it! Size was: ";
	// 		err_msg += to_string(acdc_buffer.size());
	// 		writeErrorLog(err_msg);
	// 		return 1;
	// 	}


	// 	//save this buffer a private member of ACDC
	// 	//by looping through our acdc vector
	// 	//and checking each index 
	// 	for(ACDC* a: acdcs)
	// 	{
	// 		if(a->getBoardIndex() == bi)
	// 		{
	// 			int retval;

	// 			//If raw data is requested save and return 0
	// 			if(raw==true)
	// 			{
	// 				vbuffer = acdc_buffer;
	// 				string rawfn = outfilename + "Raw_" + timestamp + "_b" + to_string(bi) + ".txt";
	// 				writeRawDataToFile(acdc_buffer, rawfn);
	// 				break;
	// 			}else
	// 			{
	// 				//parśe raw data to channel data and metadata
	// 				retval = a->parseDataFromBuffer(acdc_buffer);
	// 				map_data[bi] = a->returnData();	
	// 				if(retval == -3)
	// 				{
	// 					break;
	// 				}else if(retval == 0)
	// 				{
	// 					if(metaSwitch == 1)
	// 					{
	// 						retval = meta.parseBuffer(acdc_buffer,bi);
	// 						if(retval != 0)
	// 						{
	// 							writeErrorLog("Metadata error not parsed correctly");
	// 							return 1;						
	// 						}else
	// 						{
	// 							map_meta[bi] = meta.getMetadata();
	// 						}
	// 					}else
	// 					{
	// 						map_meta[bi] = {0};
	// 					}
	// 				}else
	// 				{
	// 					writeErrorLog("Data parsing went wrong");
	// 					return 1;
	// 				}				
	// 			}
	// 		}
	// 	}
	// }
	// if(raw==false)
	// {
	// 	datafn = outfilename + "Data_" + timestamp + ".txt";
	// 	dataofs.open(datafn.c_str(), ios::app); 
	// 	writePsecData(dataofs, boardsReadyForRead);
	// }

	return 0;
}


//------------------------------------------------------------------------------------//
//---------------------------Active functions for informations------------------------//

// >>>> ID 7: Special function to check connected ACDCs for their firmware version 
void ACC_ETH::VersionCheck()
{

    //Get ACC Info
    uint64_t acc_fw_version = eth->ReceiveDataSingle(CML.Firmware_Version_Readback,0);
    uint64_t acc_fw_date = eth->ReceiveDataSingle(CML.Firmware_Date_Readback,0);
    unsigned int acc_fw_year = (acc_fw_date & 0xffff<<16);
    unsigned int acc_fw_month = (acc_fw_date & 0xff<<8);
    unsigned int acc_fw_day = (acc_fw_date & 0xff);

    std::cout << "ACC got the firmware version: " << std::hex << acc_fw_version << std::dec;
    std::cout << " from " << std::hex << acc_fw_year << std::dec << "/" << std::hex << acc_fw_month << std::dec << "/" << std::hex << acc_fw_day << std::dec << std::endl;

    //Get ACDC Info
    for(int bi=0; bi<MAX_NUM_BOARDS; bi++)
    {
        command_address = CML.ACDC_Command;
        command_value = CML.Firmware_Version_Readback | ((1<<bi)<<24);
        uint64_t acdc_fw_version = eth->ReceiveDataSingle(command_address,command_value);

        if(acdc_fw_version==0)
        {
            std::cout << "Board " << bi << " is not connected" << std::endl;
            continue;
        }

        command_value = CML.Firmware_Date_Readback | ((1<<bi)<<24);
        uint64_t acdc_fw_date = eth->ReceiveDataSingle(command_address,command_value);
        unsigned int acdc_fw_year = (acdc_fw_date & 0xffff<<16);
        unsigned int acdc_fw_month = (acdc_fw_date & 0xff<<8);
        unsigned int acdc_fw_day = (acdc_fw_date & 0xff);

        std::cout << "Board " << bi << " got the firmware version: " << std::hex << acdc_fw_version << std::dec;
	    std::cout << " from " << std::hex << acdc_fw_year << std::dec << "/" << std::hex << acdc_fw_month << std::dec << "/" << std::hex << acdc_fw_day << std::dec << std::endl;
    }
}

//------------------------------------------------------------------------------------//
//-------------------------------------Help functions---------------------------------//

// >>>> ID 8: Fires the software trigger
void ACC_ETH::GenerateSoftwareTrigger()
{
    //Software trigger
	command_address = CML.Generate_Software_Trigger;
    command_value = 1;

    bool ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to generate a software trigger!\n",command_address,command_value);
    }
}

// >>>> ID 9: Tells ACDCs to clear their buffer
void ACC_ETH::DumpData(unsigned int boardmask)
{
    command_address = CML.RX_Buffer_Reset_Request; 
    for(unsigned int i=0; i<MAX_NUM_BOARDS; i++)
    {
        if(boardmask & (1<<i))
        {
            command_value = i;

        }
        bool ret = eth->SendData(command_address,command_value,"w");
        if(!ret)
        {
            printf("Could not send command 0x%08llX with value %i to clear RX buffer!\n",command_address,command_value);
        }
    }
}

// >>>> ID 10: Resets the ACDCs
void ACC_ETH::ResetACDC()
{
	command_address = CML.ACDC_Command;
    command_value = CML.Global_Reset | (0xff<<24) | 0x1 ;

    bool ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to reset the ACDCs!\n",command_address,command_value);
    }
}

// >>>> ID 11: Resets the ACC
void ACC_ETH::ResetACC()
{
	command_address = CML.Global_Reset;
    command_value = 0x1;

    bool ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to reset the ACC!\n",command_address,command_value);
    }
}

// >>>> ID 12: Switch PPS input to SMA
void ACC_ETH::PPStoSMA()
{
    command_address = CML.PPS_Input_Use_SMA;
    command_value = 0x1;

    bool ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to switch PPS to SMA!\n",command_address,command_value);
    }
}

// >>>> ID 13: Switch PPS input to RJ45
void ACC_ETH::PPStoRJ45()
{
    command_address = CML.PPS_Input_Use_SMA;
    command_value = 0x0;

    bool ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to switch PPS to RJ45!\n",command_address,command_value);
    }
}

// >>>> ID 14: Switch Beamgate input to SMA
void ACC_ETH::BeamgatetoSMA()
{
    command_address = CML.Beamgate_Trigger_Use_SMA;
    command_value = 0x1;

    bool ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to switch Beamgate to SMA!\n",command_address,command_value);
    }
}

// >>>> ID 15: Switch Beamgate input to RJ45
void ACC_ETH::BeamgatetoRJ45()
{
    command_address = CML.Beamgate_Trigger_Use_SMA;
    command_value = 0x0;

    bool ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to switch Beamgate to RJ45!\n",command_address,command_value);
    }
}

// >>>> ID 16: Write function for the error log
void ACC_ETH::WriteErrorLog(string errorMsg)
{
    string err = "errorlog.txt";
    cout << "------------------------------------------------------------" << endl;
    cout << errorMsg << endl;
    cout << "------------------------------------------------------------" << endl;
    ofstream os_err(err, ios_base::app);
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%m-%d-%Y %X");
    os_err << "------------------------------------------------------------" << endl;
    os_err << ss.str() << endl;
    os_err << errorMsg << endl;
    os_err << "------------------------------------------------------------" << endl;
    os_err.close();
}

