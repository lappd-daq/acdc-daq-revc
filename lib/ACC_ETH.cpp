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
    std::cout << "Connect to: " << "127.0.0.1" << ":" << "5000" << std::endl;
}

// >>>> ID:2 Constructor with IP and port arguments
ACC_ETH::ACC_ETH(std::string ip, std::string port)
{
    eth = new Ethernet(ip,port);
    std::cout << "Connect to: " << ip << ":" << port << std::endl;
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
        return -401;
    }

    //Get the connected ACDCs
    command_address = CML.ACDC_Board_Detect;
    command_value = 0;

    uint64_t DetectedBoards = eth->ReceiveDataSingle(command_address,command_value);

    if(DetectedBoards==0){return -402;}

    for(int bi=0; bi<MAX_NUM_BOARDS; bi++)
    {
        if(DetectedBoards & (1<<bi))
        {
            AcdcIndices.push_back(bi);
        }
    }

    std::cout << "Connected boards: " << AcdcIndices.size() << " at ACC-port | ";
    for(int k: AcdcIndices)
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
int ACC_ETH::ListenForAcdcData(int trigMode, vector<int> LAPPD_on_ACC)
{
	vector<int> BoardsReadyForRead;
	map<int,int> ReadoutSize;

	//setup a sigint capturer to safely
	//reset the boards if a ctrl-c signal is found
	struct sigaction sa;
	memset( &sa, 0, sizeof(sa) );
	sa.sa_handler = got_signal;
	sigfillset(&sa.sa_mask);
	sigaction(SIGINT,&sa,NULL);
  	
	//duration variables
	auto start = chrono::steady_clock::now(); //start of the current event listening. 
	auto now = chrono::steady_clock::now(); //just for initialization 
	auto printDuration = chrono::milliseconds(10000); //prints as it loops and listens
	auto lastPrint = chrono::steady_clock::now();
	auto timeoutDuration = chrono::milliseconds(timeoutvalue); // will exit and reinitialize

	while(true)
	{ 
		//Clear the boards read vector
		BoardsReadyForRead.clear(); 
		ReadoutSize.clear();
		
		//Time the listen fuction
		now = chrono::steady_clock::now();
		if(chrono::duration_cast<chrono::milliseconds>(now - lastPrint) > printDuration)
		{	
			string err_msg = "Have been waiting for a trigger for ";
			err_msg += to_string(chrono::duration_cast<chrono::milliseconds>(now - start).count());
			err_msg += " seconds";
			WriteErrorLog(err_msg);
			for(int i=0; i<MAX_NUM_BOARDS; i++)
			{
				string err_msg = "Buffer for board ";
				err_msg += to_string(i);
				err_msg += " has ";
				err_msg += to_string(LastACCBuffer.at(16+i));
				err_msg += " words";
				WriteErrorLog(err_msg);
			}
			lastPrint = chrono::steady_clock::now();
		}
		if(chrono::duration_cast<chrono::milliseconds>(now - start) > timeoutDuration)
		{
			return -601;
		}

		//If sigint happens, return value of 3
		if(quitacc.load())
		{
			return -602;
		}

        //Determine buffers and create info frame
		
		if(LastACCBuffer.size()==0)
		{
			std::cout << "ACCFRAME came up with " << LastACCBuffer.size() << std::endl;
			continue;
		}

		//go through all boards on the acc info frame and if 7795 words were transfered note that board
		for(int k: LAPPD_on_ACC)
		{

		}

		//old trigger
		if(BoardsReadyForRead==LAPPD_on_ACC)
		{
			break;
		}
	}

    //check for mixed buffersizes
    if(ReadoutSize[LAPPD_on_ACC[0]]!=ReadoutSize[LAPPD_on_ACC[1]])
    {
        std::string err_msg = "ERR: Read buffer sizes did not match: " + to_string(ReadoutSize[LAPPD_on_ACC[0]]) + " vs " + to_string(ReadoutSize[LAPPD_on_ACC[1]]);
        WriteErrorLog(err_msg);
        return -603;       
    }


	//each ACDC needs to be queried individually
	//by the ACC for its buffer. 
	for(int bi: BoardsReadyForRead)
	{
	    vector<uint64_t> acdc_buffer;

        //Here should be the read...
        command_address = CML.Read_ACDC_Data_Buffer;
        command_value = bi;
        acdc_buffer = eth->ReceiveDataVector(command_address,command_value,ReadoutSize[bi]);

		//Handles buffers =/= 7795 words
		if((int)acdc_buffer.size() != ReadoutSize[bi])
		{
			std::string err_msg = "Couldn't read " + to_string(ReadoutSize[bi]) + " words as expected! Tryingto fix it! Size was: " + to_string(acdc_buffer.size());
			WriteErrorLog(err_msg);
			return -604;
		}

		if(acdc_buffer[0] != 0x1234)
		{
			acdc_buffer.clear();
            return -604;
		}

		//raw_data.insert(raw_data.end(), acdc_buffer.begin(), acdc_buffer.end());
	}

	boardid = BoardsReadyForRead;
    BoardsReadyForRead.clear();
    
	return 0;
}


//------------------------------------------------------------------------------------//
//---------------------------Active functions for informations------------------------//

// >>>> ID 7: Special function to check connected ACDCs for their firmware version 
void ACC_ETH::VersionCheck()
{

    //Get ACC Info
    uint64_t acc_fw_version = eth->ReceiveDataSingle(CML.Firmware_Version_Readback,0x1);
    //printf("V: 0x%016llx\n",acc_fw_version);
    
    uint64_t acc_fw_date = eth->ReceiveDataSingle(CML.Firmware_Date_Readback,0x1);
    //printf("D: 0x%016llx\n",acc_fw_date);
    
    unsigned int acc_fw_year = (acc_fw_date & 0xffff<<16)>>16;
    unsigned int acc_fw_month = (acc_fw_date & 0xff<<8)>>8;
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
        unsigned int acdc_fw_year = (acdc_fw_date & 0xffff<<16)>>16;
        unsigned int acdc_fw_month = (acdc_fw_date & 0xff<<8)>>8;
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

// >>>> ID 12: Sets SMA Debug settings
void ACC_ETH::SetSMA_Debug(unsigned int PPS, unsigned int Beamgate)
{
	command_address = CML.PPS_Input_Use_SMA;
    command_value = PPS;

    bool ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to switch PPS to SMA!\n",command_address,command_value);
    }

	usleep(1000000);

    command_address = CML.Beamgate_Trigger_Use_SMA;
    command_value = Beamgate;

    ret = eth->SendData(command_address,command_value,"w");
    if(!ret)
    {
        printf("Could not send command 0x%08llX with value %i to switch Beamgate to RJ45!\n",command_address,command_value);
    }
    
	usleep(1000000);
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

