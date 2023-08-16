#ifndef _COMMANDLIBRARY_H_INCLUDED
#define _COMMANDLIBRARY_H_INCLUDED

#include <iostream>

using namespace std;


class CommandLibrary
{
    public:
        //Write
        uint64_t Global_Reset = 0x00000000;
        uint64_t RX_Buffer_Reset_Request = 0x00000002;
        uint64_t Generate_Software_Trigger = 0x00000010;
        uint64_t Read_ACDC_Data_Buffer = 0x00000020;
        uint64_t Trigger_Mode_Select = 0x00000030;
        uint64_t SMA_Polarity_Select = 0x00000038;  
        uint64_t Beamgate_Window_Start = 0x0000003A;
        uint64_t Beamgate_Window_Length = 0x0000003B;
        uint64_t PPS_Divide_Ratio = 0x0000003C;
        uint64_t PPS_Beamgate_Multiplex = 0x0000003D;
        
        //Utility
        uint64_t PPS_Input_Use_SMA = 0x00000090;
        uint64_t Beamgate_Trigger_Use_SMA = 0x00000091;
        uint64_t ACDC_Command = 0x00000100;

        //Read
        uint64_t Trigger_Mode_Readback = 0x00000030;
        uint64_t SMA_Polarity_Readback = 0x00000038;
        uint64_t Beamgate_Window_Start_Readback = 0x0000003A;
        uint64_t Beamgate_Window_Length_Readback = 0x0000003B;
        uint64_t Firmware_Version_Readback = 0x00001000;
        uint64_t Firmware_Date_Readback = 0x00001001;
        uint64_t PLL_Lock_Readback = 0x00001002;
        uint64_t ACDC_Board_Detect = 0x00001003;
        uint64_t Data_Frame_Receive = 0x00002000;
        uint64_t RX_Buffer_Empty_Readback = 0x00002001;
        uint64_t RX_Buffer_Size_Readback = 0x00002010;
        uint64_t RX_Buffer_Size_Ch0123_Readback = 0x00002019;
        uint64_t RX_Buffer_Size_Ch4567_Readback = 0x00002018;

    private:

};
#endif
