#include "ACC_ETH.h"
#include "ACC_USB.h"
#include <iostream>
#include <cstring> 

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
    if (argc <= 1)
    {
        std::cout << "Please enter an option for the connection as well:" << std::endl;
        std::cout << "./ConnectedBoards [USB or ETH]" << std::endl;
        return 0;
    }

    std::map<std::string,std::string> Settings = LoadFile();

    std::string ip = Settings["IP"];
    std::string port = Settings["Port"];

    ACC_ETH* acc_eth = nullptr;
    ACC_USB* acc_usb = nullptr;

    if(strcmp(argv[1], "USB") == 0)
    {
        acc_usb = new ACC_USB();
    }else if(strcmp(argv[1], "ETH") == 0)
    {
        acc_eth = new ACC_ETH(ip,port);
    }else
    {
        std::cout << "Please enter a valid connection option" << std::endl;
        return 0;
    }

    if(acc_usb)
    {
        acc_usb->versionCheck();
        delete acc_usb;
    }else if(acc_eth)
    {
        acc_eth->VersionCheck();
        delete acc_eth;
    }

    return 1;
}
