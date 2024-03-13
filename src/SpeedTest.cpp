#include "ACC_ETH.h"
#include "ACC_USB.h"

#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <iomanip>
#include <numeric>
#include <ctime>
#include <vector>
#include <fstream>
#include <map>
#include <chrono>

using namespace std;

ACC_ETH* acc_eth = nullptr;
ACC_USB* acc_usb = nullptr;

std::map<std::string,std::string> LoadSettings(std::string file)
{
    std::map<std::string,std::string> Settings;

    std::string line;
    std::fstream infile(file.c_str(), std::ios_base::in);
    if(!infile.is_open())
    {
        std::cout<<"File was not found! Please check for file ACC_Settings!"<<std::endl;
    }

    std::string token;
    std::vector<std::string> tokens;
    while(getline(infile, line))
    {
        if(line.empty() || line[0] == '#')
        {
            continue;
        }

        std::stringstream ss(line);
        tokens.clear();
        while(ss >> token) 
        {
            tokens.push_back((std::string)token);
        }
        Settings.insert(std::pair<std::string,std::string>(tokens[0],tokens[1]));
    }

    infile.close();
    return Settings;
}


void PrintSettings(std::map<std::string,std::string> Settings)
{
    std::cout << "------------------General settings------------------" << std::endl;
    printf("Reset ACC: %i\n",std::stoi(Settings["ResetSwitchACC"]));
    printf("Reset ACDC: %i\n",std::stoi(Settings["ResetSwitchACDC"]));
    printf("Timeout in ms: %i\n",std::stoi(Settings["Timeout"]));
    printf("ACDC boardmask: 0x%02X\n",std::stoul(Settings["ACDC_mask"],nullptr,10));
    printf("Calibration Mode: %i\n",std::stoi(Settings["Calibration_Mode"]));
    printf("SMA_PPS: %u\n",std::stoi(Settings["SMA_PPS"]));
    printf("SMA_Beamgate: %u\n",std::stoi(Settings["SMA_Beamgate"]));
    std::cout << "------------------Trigger settings------------------" << std::endl;
    printf("Triggermode: %i\n",std::stoi(Settings["Triggermode"]));
    printf("ACC trigger Sign: %i\n", std::stoi(Settings["ACC_Sign"]));
    printf("ACDC trigger Sign: %i\n", std::stoi(Settings["ACDC_Sign"]));
    printf("Selftrigger Sign: %i\n", std::stoi(Settings["SELF_Sign"]));
    printf("Coincidence Mode: %i\n", std::stoi(Settings["SELF_Enable_Coincidence"]));
    printf("Required Coincidence Channels: %u\n", std::stoi(Settings["SELF_Coincidence_Number"]));
    printf("Selftrigger threshold: %u\n", std::stoi(Settings["SELF_threshold"]));
    printf("Validation trigger start: %u ns\n", std::stoi(Settings["Validation_Start"]));
    printf("Validation trigger window: %u ns\n", std::stoi(Settings["Validation_Window"]));
    std::cout << "------------------PSEC settings------------------" << std::endl;
    printf("PSEC chipmask (chip 0 to 4) : %i|%i|%i|%i|%i\n",std::stoi(Settings["PSEC_Chip_Mask_0"]),std::stoi(Settings["PSEC_Chip_Mask_1"]),std::stoi(Settings["PSEC_Chip_Mask_2"]),std::stoi(Settings["PSEC_Chip_Mask_3"]),std::stoi(Settings["PSEC_Chip_Mask_4"]));
    printf("PSEC channelmask (for chip 0 to 4) : 0x%02X|0x%02X|0x%02X|0x%02X|0x%02X\n",std::stoul(Settings["PSEC_Channel_Mask_0"],nullptr,10),std::stoul(Settings["PSEC_Channel_Mask_1"],nullptr,10),std::stoul(Settings["PSEC_Channel_Mask_2"],nullptr,10),std::stoul(Settings["PSEC_Channel_Mask_3"],nullptr,10),std::stoul(Settings["PSEC_Channel_Mask_4"],nullptr,10));
    printf("PSEC pedestal value: %i\n", std::stoi(Settings["Pedestal_channel"]));
    printf("PSEC chipmask for pedestal: 0x%02X\n", std::stoul(Settings["Pedestal_channel_mask"],nullptr,10));
    std::cout << "------------------PSEC settings------------------" << std::endl;	
    printf("PPS divider ratio: %u\n",std::stoi(Settings["PPSRatio"]));
    printf("PPS multiplexer: %i\n",std::stoi(Settings["PPSBeamMultiplexer"]));
    std::cout << "-------------------------------------------------" << std::endl;
}


void StartTest_USB(std::map<std::string,std::string> Settings, int NumOfEvents)
{
    acc_usb->setTimeoutInMs(std::stoi(Settings["Timeout"]));

	acc_usb->setSign(std::stoi(Settings["ACC_Sign"]), 2);
	acc_usb->setSign(std::stoi(Settings["ACDC_Sign"]), 3);
	acc_usb->setSign(std::stoi(Settings["SELF_Sign"]), 4);

    acc_usb->setEnableCoin(std::stoi(Settings["SELF_Enable_Coincidence"]));
    acc_usb->setNumChCoin(std::stoi(Settings["SELF_Coincidence_Number"]));
    acc_usb->setThreshold(std::stoi(Settings["SELF_threshold"]));

    std::vector<int> PsecChipMask = {std::stoi(Settings["PSEC_Chip_Mask_0"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_1"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_2"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_3"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_4"])};
	std::vector<unsigned int> VecPsecChannelMask = {static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_0"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_1"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_2"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_3"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_4"],nullptr,10))};
    acc_usb->setPsecChipMask(PsecChipMask);
	acc_usb->setPsecChannelMask(VecPsecChannelMask);

    acc_usb->setValidationStart((unsigned int)std::stoi(Settings["Validation_Start"])/25); 
    acc_usb->setValidationWindow((unsigned int)std::stoi(Settings["Validation_Window"])/25);

    acc_usb->setPedestals(std::stoul(Settings["ACDC_mask"],nullptr,10),std::stoul(Settings["Pedestal_channel_mask"],nullptr,10),std::stoi(Settings["Pedestal_channel"]));

    acc_usb->setPPSRatio(std::stoi(Settings["PPSRatio"]));
    acc_usb->setPPSBeamMultiplexer(std::stoi(Settings["PPSBeamMultiplexer"]));

    acc_usb->setSMA_Debug(std::stoi(Settings["SMA_PPS"]),std::stoi(Settings["SMA_Beamgate"]));

    int retval;
	retval = acc_usb->initializeForDataReadout(std::stoi(Settings["Triggermode"]), std::stoul(Settings["ACDC_mask"],nullptr,10), std::stoi(Settings["Calibration_Mode"]));
    if(retval!=0)
    {
        std::cout << "Setup went wrong!" << std::endl;
    }

    int events = 0;
    int read_back = -1;
    vector<int> LAPPD_on_ACC = {std::stoi(Settings["Port0"])};
    if(std::stoi(Settings["Port0"]) != std::stoi(Settings["Port1"]))
    {
        LAPPD_on_ACC.push_back(std::stoi(Settings["Port1"]));
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    ofstream outfile("./datafile_usb.txt", ios_base::out | ios_base::trunc);
    while(events<NumOfEvents)
    {
        if(Settings["Triggermode"]=="1")
        {
            acc_usb->softwareTrigger();
        }
        read_back = acc_usb->listenForAcdcData(std::stoi(Settings["Triggermode"]),LAPPD_on_ACC);
        if(read_back==0)
        {
            vector<unsigned short> data = acc_usb->returnRaw();
            vector<unsigned short> accif = acc_usb->returnACCIF();
            vector<int> bi = acc_usb->returnBoardIndices();
            acc_usb->clearData();
	    
	    for(unsigned short k: data)
	    {
		outfile << k << "\t";
	    }
	    outfile << endl;
	     
            events++;
        }else
        {
            if(read_back==404)
            {
                //pass  
            }else
            {
                std::cout << "A non-404 error happened: " << read_back << std::endl;
                acc_usb->clearData();
                acc_usb->dumpData(0xff);
                acc_usb->emptyUsbLine();
            }
        }
    }
    outfile.close();
    auto t1 = std::chrono::high_resolution_clock::now();
	auto dt = 1.e-9*std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
	cout << "It took "<< dt <<" second(s)."<< endl;
    cout << "The mean rate was thus "<< 1/dt <<" Hz."<< endl;
}

void StartTest_ETH_buffercheck(std::map<std::string,std::string> Settings, int NumOfEvents)
{
    acc_eth->SetTimeoutInMs(std::stoi(Settings["Timeout"]));

	acc_eth->SetSign(std::stoi(Settings["ACC_Sign"]), 2);
	acc_eth->SetSign(std::stoi(Settings["ACDC_Sign"]), 3);
	acc_eth->SetSign(std::stoi(Settings["SELF_Sign"]), 4);

    acc_eth->SetEnableCoin(std::stoi(Settings["SELF_Enable_Coincidence"]));
    acc_eth->SetNumChCoin(std::stoi(Settings["SELF_Coincidence_Number"]));
    acc_eth->SetThreshold(std::stoi(Settings["SELF_threshold"]));

    std::vector<int> PsecChipMask = {std::stoi(Settings["PSEC_Chip_Mask_0"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_1"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_2"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_3"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_4"])};
	std::vector<unsigned int> VecPsecChannelMask = {static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_0"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_1"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_2"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_3"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_4"],nullptr,10))};
    acc_eth->SetPsecChipMask(PsecChipMask);
	acc_eth->SetPsecChannelMask(VecPsecChannelMask);

    acc_eth->SetValidationStart((unsigned int)std::stoi(Settings["Validation_Start"])/25); 
    acc_eth->SetValidationWindow((unsigned int)std::stoi(Settings["Validation_Window"])/25);

    acc_eth->SetPedestals(std::stoul(Settings["ACDC_mask"],nullptr,10),std::stoul(Settings["Pedestal_channel_mask"],nullptr,10),std::stoi(Settings["Pedestal_channel"]));

    acc_eth->SetPPSRatio(std::stoi(Settings["PPSRatio"]));
    acc_eth->SetPPSBeamMultiplexer(std::stoi(Settings["PPSBeamMultiplexer"]));

    acc_eth->SetSMA_Debug(std::stoi(Settings["SMA_PPS"]),std::stoi(Settings["SMA_Beamgate"]));

    int retval;
	retval = acc_eth->InitializeForDataReadout(std::stoul(Settings["ACDC_mask"],nullptr,10), std::stoi(Settings["Triggermode"]));
    if(retval!=0)
    {
        std::cout << "Setup went wrong!" << std::endl;
    }else
    {
        std::cout << "Setup went well!" << std::endl;
    }

    int events = 0;
    int read_back = -1;
    vector<int> LAPPD_on_ACC = {std::stoi(Settings["Port0"])};
    if(std::stoi(Settings["Port0"]) != std::stoi(Settings["Port1"]))
    {
        LAPPD_on_ACC.push_back(std::stoi(Settings["Port1"]));
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    while(events<NumOfEvents)
    {
        int doomcount = 0;
        if(Settings["Triggermode"]=="1")
        {
            std::cout<<"Software trigger" <<std::endl;
            acc_eth->GenerateSoftwareTrigger();
        }

        std::vector<uint64_t> ret_acc = {601};
        while(ret_acc.at(0)==601 && doomcount<10)
        {
            ret_acc = acc_eth->Temp_Read(std::stoi(Settings["Triggermode"]),LAPPD_on_ACC);
            doomcount++;
        }

        events++;
        acc_eth->ClearData();
    }
}


void StartTest_ETH(std::map<std::string,std::string> Settings, int NumOfEvents)
{
    acc_eth->SetTimeoutInMs(std::stoi(Settings["Timeout"]));

	acc_eth->SetSign(std::stoi(Settings["ACC_Sign"]), 2);
	acc_eth->SetSign(std::stoi(Settings["ACDC_Sign"]), 3);
	acc_eth->SetSign(std::stoi(Settings["SELF_Sign"]), 4);

    acc_eth->SetEnableCoin(std::stoi(Settings["SELF_Enable_Coincidence"]));
    acc_eth->SetNumChCoin(std::stoi(Settings["SELF_Coincidence_Number"]));
    acc_eth->SetThreshold(std::stoi(Settings["SELF_threshold"]));

    std::vector<int> PsecChipMask = {std::stoi(Settings["PSEC_Chip_Mask_0"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_1"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_2"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_3"]),
                                        std::stoi(Settings["PSEC_Chip_Mask_4"])};
	std::vector<unsigned int> VecPsecChannelMask = {static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_0"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_1"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_2"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_3"],nullptr,10)),
                                                    static_cast<unsigned int>(std::stoul(Settings["PSEC_Channel_Mask_4"],nullptr,10))};
    acc_eth->SetPsecChipMask(PsecChipMask);
	acc_eth->SetPsecChannelMask(VecPsecChannelMask);

    acc_eth->SetValidationStart((unsigned int)std::stoi(Settings["Validation_Start"])/25); 
    acc_eth->SetValidationWindow((unsigned int)std::stoi(Settings["Validation_Window"])/25);

    acc_eth->SetPedestals(std::stoul(Settings["ACDC_mask"],nullptr,10),std::stoul(Settings["Pedestal_channel_mask"],nullptr,10),std::stoi(Settings["Pedestal_channel"]));

    acc_eth->SetPPSRatio(std::stoi(Settings["PPSRatio"]));
    acc_eth->SetPPSBeamMultiplexer(std::stoi(Settings["PPSBeamMultiplexer"]));

    acc_eth->SetSMA_Debug(std::stoi(Settings["SMA_PPS"]),std::stoi(Settings["SMA_Beamgate"]));

    int retval;
	retval = acc_eth->InitializeForDataReadout(std::stoul(Settings["ACDC_mask"],nullptr,10), std::stoi(Settings["Triggermode"]));
    if(retval!=0)
    {
        std::cout << "Setup went wrong!" << std::endl;
    }else
    {
        std::cout << "Setup went well!" << std::endl;
    }

    int events = 0;
    int read_back = -1;
    vector<int> LAPPD_on_ACC = {std::stoi(Settings["Port0"])};
    if(std::stoi(Settings["Port0"]) != std::stoi(Settings["Port1"]))
    {
        LAPPD_on_ACC.push_back(std::stoi(Settings["Port1"]));
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    while(events<NumOfEvents)
    {
        if(Settings["Triggermode"]=="1" && read_back!=-601)
        {
            std::cout<<"Software trigger" <<std::endl;
            acc_eth->GenerateSoftwareTrigger();
        }else if (Settings["Triggermode"]=="1" && read_back==-601)
        {
            std::cout << "Since timeout was called is will not trigger again" << std::endl;
        }
        
        read_back = acc_eth->ListenForAcdcData(std::stoi(Settings["Triggermode"]),LAPPD_on_ACC);
        if(read_back==0)
        {
            std::cout<<"Got good data "<<std::endl;
            vector<unsigned short> data = acc_eth->ReturnRawData();
            vector<uint64_t> accif = acc_eth->ReturnACCIF();
            vector<int> bi = acc_eth->ReturnBoardIndices();
            acc_eth->ClearData();
            if(data.size()==7795+16)
            {
                std::string name = "./MixedFrame.txt";
                ofstream file(name.c_str(),ios_base::out | ios_base::trunc);
                for(int k=0; k<data.size(); k++)
                {
                    file<<k<<" "<<data.at(k)<<endl;
                }
                file.close();
            }
            if(data.size()==2*7795+2)
            {
                std::string name = "./DataFrame.txt";
                ofstream file(name.c_str(),ios_base::out | ios_base::trunc);
                for(int k=0; k<data.size(); k++)
                {
                    file<<k<<" "<<std::hex<<data.at(k)<<std::dec<<endl;
                }
                file.close();
            }
            events++;
        }else
        {
            if(read_back==-601)
            {
                //pass  
            }else if(read_back==-602)
            {
                exit(1);
            }else
            {
                std::cout << "A non-404 error happened: " << read_back << std::endl;
                exit(1);
                acc_eth->ClearData();
                acc_eth->DumpData(0xff);
            }
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
	auto dt = 1.e-9*std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
	cout << "It took "<< dt <<" second(s)."<< endl;
    cout << "The mean rate was thus "<< NumOfEvents/dt <<" Hz."<< endl;
}


int main(int argc, char *argv[])
{
    //Load ACC 
    if(argc <= 2)
    {
        std::cout << "Please enter an option for the connection as well:" << std::endl;
        std::cout << "./SpeedTest [USB or ETH] [Number of Events] [*options]" << std::endl;
        return 0;
    }

    //Load All ACC Settings
    std::map<std::string,std::string> Settings = LoadSettings("./ACC_Settings");
    PrintSettings(Settings);

    std::map<std::string,std::string> Settings_Connection = LoadSettings("./ConnectionSettings");

    std::string ip = Settings_Connection["IP"];
    std::string port = Settings_Connection["Port"];

    std::string choice_yn;
    while(true)
    {
        if(argc>3)
        {
            if(strcmp(argv[3], "-y") == 0)
            {
                break;
            }else
            {
                std::cout << argv[3] << std::endl;
            }
        }
        std::cout << "Are you ok with these settings (y/n)?   ";
        std::cin >> choice_yn;
        if(choice_yn=="y")
        {
            break;
        }else if(choice_yn=="n")
        {
            return 0;
        }
    }

    if(strcmp(argv[1], "USB") == 0)
    {
        acc_usb = new ACC_USB();
    }else if(strcmp(argv[1], "ETH") == 0 || strcmp(argv[1], "ETH_TEST") == 0)
    {
        acc_eth = new ACC_ETH(ip,port);
    }else
    {
        std::cout << "Please enter a valid connection option" << std::endl;
        return 0;
    }

    int NumOfEvents = std::stoi(argv[2]);
    if(strcmp(argv[1], "USB") == 0 && acc_usb)
    {
        StartTest_USB(Settings, NumOfEvents);
    }else if(strcmp(argv[1], "ETH") == 0 && acc_eth)
    {
        StartTest_ETH(Settings, NumOfEvents);
    }else if(strcmp(argv[1], "ETH_TEST") == 0&& acc_eth)
    {
        StartTest_ETH_buffercheck(Settings, NumOfEvents);
    }else
    {
        std::cout << "Please enter a valid connection option" << std::endl;
        return 0;
    }

    delete acc_eth;
    delete acc_usb;

    return 1;
}
