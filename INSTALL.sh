#!/bin/bash

#Colors
TEXT='\033[0;32m'
NC='\033[0m'
VALUES='\033[1;33m'
CODE='\033[0;36m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'

FOX_ASCII=$(cat << "EOF"
   /\   /\   
  //\\_//\\     ____
  \_     _/    /   /
   / * * \    /^^^]
   \_\O/_/    [   ]            _/_/      _/_/_/    _/_/_/   
    /   \_    [   /         _/    _/  _/        _/          
    \     \_  /  /         _/_/_/_/  _/        _/      
     [ [ /  \/ _/         _/    _/  _/        _/       
    _[ [ \  /_/          _/    _/    _/_/_/    _/_/_/   
EOF
)

ANNIE_ASCII=$(cat << "EOF"
             ,@@              
            /@@@@,            
           gM ]@@@y ,&$&&&W,  
          @C   ^@@k&@@@%%@@@& 
         @`      @$&@H@@@@@@&L
       ,@         B&@@@@H@@@& 
 ,m@@md@           ]&&@@@@&M            _/_/      _/_/_/    _/_/_/ 
#HHHHHH[            ]kkk[            _/    _/  _/        _/ 
]HHHHHHM             @@@@           _/_/_/_/  _/        _/  
 `*MMH[             ]@@@@          _/    _/  _/        _/  
     ]@,           ,@@@@C         _/    _/    _/_/_/    _/_/_/ 
      ^@@w       ,@@@@@"      
        *%@@@@@@@@@@%"        
            "****"                                   
EOF
)

display_fox() {
    echo "$FOX_ASCII"
}

display_annie() {
    echo "$ANNIE_ASCII"
}

echo ""

#display_fox
display_annie

echo ""

#Check and install prerequisites
echo -e ">>>> ${TEXT}Checking prerequisites... ${NC}"
#Do that
# Function to check if a package is installed
is_package_installed() {
    dpkg -s "$1" &> /dev/null
}

# Install libusb-1.0 if not already installed
if ! is_package_installed g++; then
    echo "g++ is not installed. Installing..."
    sudo apt-get update
    sudo apt-get install -y g++
    echo "g++ has been installed."
else
    echo "g++ is already installed."
fi
if ! is_package_installed cmake; then
    echo "cmake is not installed. Installing..."
    sudo apt-get update
    sudo apt-get install -y cmake
    echo "cmake has been installed."
else
    echo "cmake is already installed."
fi
if ! is_package_installed libusb-1.0-0; then
    echo "libusb-1.0 is not installed. Installing..."
    sudo apt-get update
    sudo apt-get install -y libusb-1.0-0-dev libusb-1.0-0
    echo "libusb-1.0 has been installed."
else
    echo "libusb-1.0 is already installed."
fi
echo ""

#Get the amount of cpus available for compiling
num_cpu=$(grep -c ^processor /proc/cpuinfo)
use_cpu=$((num_cpu/2))
echo -e ">>>> ${TEXT}Got ${VALUES}$num_cpu${TEXT} CPU cores, will use ${VALUES}$use_cpu${TEXT} for compiling${NC}"
echo ""

#Create build folder and files
echo -e ">>>> ${TEXT}Executing ${CODE}cmake . -Bbuild${NC}"
cmake . -Bbuild
echo ""

#Compile
echo -e ">>>> ${TEXT}Executing ${CODE}cmake --build build -- -j${VALUES}$use_cpu${NC}"
cmake --build build -- -j$use_cpu 
