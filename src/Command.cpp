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
    if(argc < 4)
    {
        std::cout << "Usage: ./Command [command address] [command value] [read/write]" << std::endl;
        std::cout << "[command address]: Address for the RX command" << std::endl;
        std::cout << "[command value]: Value for the RX command" << std::endl;
        std::cout << "[read/write]: Read single/vector or write command selected by rs, rv or w" << std::endl;

        return 0;
    }

    std::map<std::string,std::string> Settings = LoadFile();

    std::string ip = Settings["IP"];
    std::string port = Settings["Port"];
    std::string rw = argv[3];
    std::string command = argv[1];
    std::string value = argv[2];

    uint64_t c_addr = strtoull(command.c_str(), NULL, 16);
    uint64_t c_value = strtoull(value.c_str(), NULL, 16);

    std::cout << "Connect to: " << ip << ":" << port << std::endl;
    Ethernet *eth = new Ethernet(ip,port);

    if(rw=="rv" || rw=="rs" || rw=="rb")
    {
        if(rw=="rv")
        {
            std::vector<uint64_t> returndata = eth->ReceiveDataVector(c_addr,c_value,-1);
            for(uint64_t k: returndata)
            {
                std::cout << std::hex << k << std::dec << std::endl;
            }
        }else if(rw=="rs")
        {
            uint64_t returndata = eth->ReceiveDataSingle(c_addr,c_value);
            printf("Received: 0x%016llx\n",returndata);
        }else
        {
            std::vector<uint64_t> returndata = eth->RecieveBurst(7795);
            for(uint64_t k: returndata)
            {
                std::cout << std::hex << k << std::dec << std::endl;
            }  
        }
    }else if(rw=="w")
    {
        eth->SendData(c_addr,c_value,rw);
    }else
    {
        std::cout << "Invalid read or write command chosen: " << rw << std::endl;
    }

    return 1;
}
