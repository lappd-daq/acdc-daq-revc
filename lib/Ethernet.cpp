#include "Ethernet.h"

Ethernet::Ethernet(std::string ipaddr, std::string port) : m_socket(-1), servinfo(NULL)
{
    bool ret = OpenInterface(ipaddr, port);
    if(ret==false)
    {
        std::cerr << "Failed to open the interface ending program" << std::endl;
        CloseInterface();
        exit(EXIT_FAILURE);
    }else
    {
        std::cout << "Connected to the ACC" << std::endl;
    }
}

Ethernet::~Ethernet()
{
    CloseInterface();
}

bool Ethernet::OpenInterface(std::string ipaddr, std::string port)
{
    struct addrinfo  hints;
    int s;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if((s = getaddrinfo(ipaddr.c_str(), port.c_str(), &hints, &servinfo)) != 0)
    {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return false;
    }

    // loop through all the results and make a socket
    for(list_of_addresses = servinfo; list_of_addresses != NULL; list_of_addresses = list_of_addresses->ai_next)
    {
        m_socket = socket(list_of_addresses->ai_family, list_of_addresses->ai_socktype, list_of_addresses->ai_protocol);
        if(m_socket == -1){continue;}

        //if(coonnect(m_socket, list_of_addresses->ai_addr, list_of_addresses->ai_addrlen)!=-1)
        //{
            break;
        //}
        //close(m_socket);
    }

    if(list_of_addresses == NULL || m_socket==-1)
    {
	    fprintf(stderr, "Failed to create socket\n");
        return false;
    }
    
    
    FD_ZERO(&rfds_);
    FD_SET(m_socket, &rfds_);

    return true;
}

void Ethernet::CloseInterface()
{
    if(m_socket>0) 
    {
        freeaddrinfo(servinfo);
        close(m_socket);
        m_socket = 0;
        std::cout << "Disconnected from ACC" << std::endl;
    }
}

bool Ethernet::SendData(uint32_t addr, uint64_t value, std::string read_or_write) 
{
    if(m_socket<=0) 
    {
        std::cerr << "Socket not open." << std::endl;
        return false;
    }

    int rw = -1;
    if(read_or_write=="r")
    {
        rw = 0;
    }else if(read_or_write=="w")
    {
        rw = 1;
    }else
    {
        return false;
    }


    buffer[0] = rw;
    buffer[1] = 1;

    //Make command from in
    memcpy(&buffer[RX_ADDR_OFFSET_], &addr, 4);
    memcpy(&buffer[RX_DATA_OFFSET_], &value, 8);

    int packet_size = RX_DATA_OFFSET_ + 8;

    int returnval = sendto(m_socket,buffer,packet_size,0, list_of_addresses->ai_addr, list_of_addresses->ai_addrlen);
    if(returnval==-1)
    {
        std::cout << "Error data not send, tried to send " << buffer << std::endl; 
    }else
    {
        printf("Data was sucessfully sent, sent was 0x%016llx to 0x%08llx with %i bytes\n",value,addr,returnval);
    }

    return true;
}

std::vector<unsigned short> Ethernet::ReceiveDataVector(uint32_t addr, uint64_t value, int size) 
{
    if(m_socket<=0) 
    {
        std::cerr << "Socket not open." << std::endl;
        return {};
    }

    if(!SendData(addr,value,"r"))
    {
        std::cout << "Could not send read command" << std::endl;
        return {};
    }

    if(size==-1)
    {
        size = 16000;
    }

    std::vector<uint64_t> return_buffer;

    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof(their_addr);

    int rec_bytes=-1;
    while((int)return_buffer.size()!=size)
    {
        rec_bytes = recvfrom(m_socket,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&their_addr,&addr_len);
        if(rec_bytes<0)
        {
            std::cout << "Could not receive data! Got " << rec_bytes << " bytes" << std::endl;
        }else
        {
            uint64_t data;
            memcpy((void*)&data, (void*)&buffer[TX_DATA_OFFSET_], 8);
            return_buffer.push_back(data);
        }   
    }

    if(size==7795)
    {
        return_buffer.erase(return_buffer.begin());
    }

    std::vector<unsigned short> converted_buffer;
    for(const uint64_t conv_data : return_buffer) 
    {
        converted_buffer.push_back(static_cast<unsigned short>(conv_data));
    }
    return converted_buffer;
}

uint64_t Ethernet::ReceiveDataSingle(uint32_t addr, uint64_t value) 
{
    if(m_socket<=0) 
    {
        std::cerr << "Socket not open." << std::endl;
        return {};
    }

    int rec_bytes = -1;
	
    if(!SendData(addr,value,"r"))
    {
        std::cout << "Could not send read command" << std::endl;
        return {};
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof(their_addr);
    
    tv_ = {0, 250000};  // 0 seconds and 250000 useconds
    int retval = select(m_socket+1, &rfds_, NULL, NULL, &tv_);

    if(retval > 0)
    {
        if((rec_bytes = recvfrom(m_socket,
                                buffer,
                                MAXBUFLEN_ - 1,
                                0,
                                (struct sockaddr*)&their_addr,
                                &addr_len)) == -1)
        {
            //perror("recvfrom");
            return 406;
        }

        if(packetID_ >= 0 && (packetID_ + 1)%256 != buffer[1]) printf("Missing packet? We jumped from packet id %d to %d\n", packetID_,buffer[1]);
        packetID_ = buffer[1];

    }
    else if(retval == 0)
    {
        printf("Read Timeout\n");
        return 405;
    }
    else
    {
        perror("select()");
        return 404;
    }

    
    uint64_t data;
    if(rec_bytes<0)
    {
        std::cout << "Could not receive data! Got " << rec_bytes << " bytes" << std::endl;
    }else
    {
        memcpy((void*)&data, (void*)&buffer[TX_DATA_OFFSET_], 8);
    }   
    
    return data;
}