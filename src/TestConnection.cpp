#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

#include "Ethernet.h"

using namespace std;

std::map<std::string,std::string> LoadFile()
{
    std::map<std::string,std::string> Settings;

    std::string line;
    std::fstream infile("./ConnectionSettings", std::ios_base::in);
    if(!infile.is_open())
    {
        std::cout<<"File was not found! Please check for file ConnectionSettings!"<<std::endl;
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

int main(int argc, char *argv[])
{
    std::cout << "Testing Connection ... " << std::endl;

    std::vector<int> connections;

    std::string cred = "\033[1;31m";
    std::string cgreen = "\033[1;32m";
    std::string cnormal = "\033[0m";

    std::map<std::string,std::string> Settings = LoadFile();

    std::string ip = Settings["IP"];
    std::string port = Settings["Port"];

    //std::cout << ">>>> Connecting to: " << ip << ":" << port << std::endl;

    int mode = 0;
    if(argc==2){mode=std::stoi(argv[1]);}
    
    if(mode==1)
    {
	    for(int n_port=0; n_port<100000; n_port++)
	    {
	    	std::cout<<"Testing port "<<n_port<<std::endl;
		port = to_string(n_port).c_str();
	    	Ethernet *eth = new Ethernet(ip,port);

	    	uint64_t ret = eth->RecieveDataSingle(0x00001000, 0x0);
		if(ret!=404 && ret !=405 && ret!=406)
		{
			std::cout << cgreen << ">>>> SUCCESS" << cnormal << std::endl;
			connections.push_back(n_port);
	    	}else
	    	{
			std::cout << cred << ">>>> FAILED with " << ret << cnormal << std::endl;
	    	}
	    	delete eth;
	    	usleep(10000);
	    }	
	    std::cout<<"Successfull ports: "<<std::endl;
	    for(int k: connections){std::cout<<k<<std::endl;}
    }else if(mode==2)
    {
	Ethernet *eth = new Ethernet(ip,port);

    	uint64_t ret = eth->RecieveDataSingle(0x00001001, 0x0);
    	if(ret==404 || ret==405 || ret==406){return 0;}
    	printf("Data: 0x%016llx\n",ret);
        unsigned int acc_fw_year = (ret & 0xffff<<16)>>16;
	unsigned int acc_fw_month = (ret & 0xff<<8)>>8;
    	unsigned int acc_fw_day = (ret & 0xff);
    	std::cout << " from " << std::hex << acc_fw_year << std::dec << "/" << std::hex << acc_fw_month << std::dec << "/" << std::hex << acc_fw_day << std::dec << std::endl;
    }
    return 1;
}
