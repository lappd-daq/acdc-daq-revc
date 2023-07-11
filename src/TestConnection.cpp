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

    std::string cred = "\033[1;31m";
    std::string cgreen = "\033[1;32m";
    std::string cnormal = "\033[0m";

    std::map<std::string,std::string> Settings = LoadFile();

    std::string ip = Settings["IP"];
    std::string port = Settings["Port"];

    std::cout << ">>>> Connecting to: " << ip << ":" << port << std::endl;

    Ethernet *eth = new Ethernet(ip,port);

    bool ret = eth->SendData(0x00001000, 0, "w");
    if(ret)
    {
        std::cout << cgreen << ">>>> SUCCESS" << cnormal << std::endl;
    }else
    {
        std::cout << cred << ">>>> FAILED" << cnormal << std::endl;
    }
}
