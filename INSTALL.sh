#!/bin/bash

#Colors
TEXT='\033[0;32m'
NC='\033[0m'
VALUES='\033[1;33m'
CODE='\033[0;36m'

#Check and install prerequisites
echo -e ">>>> ${TEXT}Checking prerequisites... ${NC}"
#Do that
# Function to check if a package is installed
is_package_installed() {
    dpkg -s "$1" &> /dev/null
}

# Install libusb-1.0 if not already installed
if ! is_package_installed libusb-1.0-0; then
    echo "libusb-1.0 is not installed. Installing..."
    sudo apt-get update
    sudo apt-get install -y libusb-1.0-0
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