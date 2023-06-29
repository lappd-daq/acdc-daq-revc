#include "Ethernet.h"

using namespace std;

int main(int argc, char *argv[])
{
    if(argc < 5)
    {
        std::cout << "Usage: ./Command [ip adress] [port] [read/write] [command address] [command value]" << std::endl;
        std::cout << "[ip address]: IP address of the ACC connection" << std::endl;
        std::cout << "[port]: Port of the ACC conection" << std::endl;
        std::cout << "[read/write]: Read single/vector or write command selected by rs, rv or w" << std::endl;
        std::cout << "[command address]: Address for the RX command" << std::endl;
        std::cout << "[command value]: Value for the RX command (optional, default = 0)" << std::endl;

        return 0;
    }else if(argc == 5)
    {
        argv[4] = 0;
    }

    std::string ip = argv[1];
    std::string port = argv[2];
    std::string rw = argv[3];
    std::string command = argv[4];
    std::string value = argv[5];

    std:cout << "Connect to: " << ip << ":" << port << std::endl;
    Ethernet *eth = new Ethernet(ip,port);

    if(rw=="rv" || rw=="rs")
    {
        if(rw=="rs")
        {
            std::vector<unsigned short> returndata = eth->ReceiveDataVector(std::stoul(command),std::stoul(value),-1);
            for(unsigned short k: returndata)
            {
                std::cout << std::hex << k << std::dec << std::endl;
            }
        }else
        {
            unsigned int returndata = eth->ReceiveDataSingle(std::stoul(command),std::stoul(value),-1);
        }
    }else if(rw=="w")
    {
        eth->SendData(std::stoul(command),std::stoul(value),rw);
    }else
    {
        std::cout << "Invalid read or write command chosen: " << rw << std::endl;
    }

    return 1;
}