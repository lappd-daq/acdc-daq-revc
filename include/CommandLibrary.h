#ifndef _COMMANDLIBRARY_H_INCLUDED
#define _COMMANDLIBRARY_H_INCLUDED

#include <iostream>

using namespace std;


class CommandLibrary
{
    public:
        //Write
        unsigned int Global_Reset = 0x00000000;
        unsigned int RX_Buffer_Reset_Request = 0x00000002;
        unsigned int Generate_Software_Trigger = 0x00000010;
        unsigned int Read_ACDC_Data_Buffer = 0x00000020;
        unsigned int Trigger_Mode_Select = 0x00000030;
        unsigned int SMA_Polarity_Select = 0x00000038;  
        unsigned int Beamgate_Window_Start = 0x0000003A;
        unsigned int Beamgate_Window_Length = 0x0000003B;
        unsigned int PPS_Divide_Ratio = 0x0000003C;
        unsigned int PPS_Beamgate_Multiplex = 0x0000003D;
        
        //Utility
        unsigned int PPS_Input_Use_SMA = 0x00000090;
        unsigned int Beamgate_Trigger_Use_SMA = 0x00000091;
        unsigned int ACDC_Command = 0x00000100;

        //Read
        unsigned int Trigger_Mode_Readback = 0x00000030;
        unsigned int SMA_Polarity_Readback = 0x00000038;
        unsigned int Beamgate_Window_Start_Readback = 0x0000003A;
        unsigned int Beamgate_Window_Length_Readback = 0x0000003B;
        unsigned int Firmware_Version_Readback = 0x00001000;
        unsigned int Firmware_Date_Readback = 0x00001001;
        unsigned int PLL_Lock_Readback = 0x00001002;
        unsigned int ACDC_Board_Detect = 0x00001003;
        unsigned int Data_Frame_Receive = 0x00002000;
        unsigned int RX_Buffer_Empty_Readback = 0x00002001;
        unsigned int RX_Buffer_Size_Readback = 0x00002010;
        unsigned int RX_Buffer_Size_Ch0123_Readback = 0x00002019;
        unsigned int RX_Buffer_Size_Ch4567_Readback = 0x00002018;

    private:

};
#endif