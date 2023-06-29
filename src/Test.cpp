#include <iostream>
#include "Ethernet.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello World" << std::endl;

    std::string ip = argv[1];
    std::string port = argv[2];

    std:cout << "Connect to: " << ip << ":" << port << std::endl;

    Ethernet *eth = new Ethernet(ip,port);

    eth->SendData(0x00001000, 0, 0);
}

/*
// >>>> ID xx: Write function for the raw data format
void ACC::WriteRawDataToFile(vector<unsigned short> buffer, string rawfn)
{
	ofstream d(rawfn.c_str(), ios::app); 
	for(unsigned short k: buffer)
	{
		d << hex <<  k << " ";
	}
	d << endl;
	d.close();
	return;
}

// >>>> ID xx: Write function for the parsed data format
void ACC::WritePsecData(ofstream& d, vector<int> boardsReadyForRead)
{
	string delim = " ";
	for(int enm=0; enm<NUM_SAMP; enm++)
	{
		d << dec << enm << delim;
		for(int bi: boardsReadyForRead)
		{
			if(map_data[bi].size()==0)
			{
				cout << "Mapdata is empty" << endl;
				writeErrorLog("Mapdata empty");
			}
			for(int ch=0; ch<NUM_CH; ch++)
			{
				if(enm==0)
				{
					//cout << "Writing board " << bi << " and ch " << ch << ": " << map_data[bi][ch+1][enm] << endl;
				}
				d << dec << (unsigned short)map_data[bi][ch][enm] << delim;
			}
			if(enm<(int)map_meta[bi].size())
			{
				d << hex << map_meta[bi][enm] << delim;

			}else
			{
				d << 0 << delim;
			}
		}
		d << endl;
	}
	d.close();
}

*/